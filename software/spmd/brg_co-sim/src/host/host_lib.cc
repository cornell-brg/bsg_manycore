#include <stdint.h>
#include <stdio.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "ipc.h"
#include "host_lib.h"

using namespace boost::interprocess;

int device_malloc(void** dev_ptr, size_t size)
{
  message_queue req_q(open_only, g_req_msg_q_name);
  message_queue resp_q(open_only, g_resp_msg_q_name);

  unsigned int priority = 0;
  message_queue::size_type recvd_size;

  // prepare request
  IPC_Request req;
  req.type = IPC_Request_Type_Malloc;
  req.payloads[0] = size;

  // send request
  req_q.send(&req, sizeof(req), priority);

  // receive response
  IPC_Response resp;
  resp_q.receive(&resp, sizeof(resp), recvd_size, priority);
  // printf("%s: response = %x\n", __FILE__, resp.payloads[0]);
  *dev_ptr = (void*)resp.payloads[0];
  return 1;
}

int memcpy_host_to_device(const void* dev_dst, const void* src, size_t size)
{
  message_queue req_q(open_only, g_req_msg_q_name);
  message_queue resp_q(open_only, g_resp_msg_q_name);
  shared_memory_object shm (open_only, g_shared_mem_name, read_write);
  mapped_region region(shm, read_write);

  // first copy content into IPC shared memory region
  memcpy(region.get_address(), src, size);

  unsigned int priority = 0;
  message_queue::size_type recvd_size;

  // prepare request
  IPC_Request req;
  req.type = IPC_Request_Type_MemcpyHostToDevice;
  req.payloads[0] = (uint64_t)dev_dst;
  req.payloads[1] = (uint64_t)size;

  // send request
  req_q.send(&req, sizeof(req), priority);

  // receive response
  IPC_Response resp;
  resp_q.receive(&resp, sizeof(resp), recvd_size, priority);
  return resp.payloads[0];
}

int memcpy_device_to_host(const void* dev_src, void* dest, size_t size)
{
  message_queue req_q(open_only, g_req_msg_q_name);
  message_queue resp_q(open_only, g_resp_msg_q_name);
  shared_memory_object shm (open_only, g_shared_mem_name, read_write);
  mapped_region region(shm, read_write);

  unsigned int priority = 0;
  message_queue::size_type recvd_size;

  // prepare request
  IPC_Request req;
  req.type = IPC_Request_Type_MemcpyDeviceToHost;
  req.payloads[0] = (uint64_t)dev_src;
  req.payloads[1] = (uint64_t)size;

  // send request
  req_q.send(&req, sizeof(req), priority);

  // receive response
  IPC_Response resp;
  resp_q.receive(&resp, sizeof(resp), recvd_size, priority);
  int* shm_ptr = static_cast<int*>(region.get_address());
  const void* host_dst = shm_ptr;//(void*)resp.payloads[0];
  size_t src_size = (size_t) resp.payloads[1] * sizeof(int);
  // const uint64_t addr = (resp.payloads[0]);
  // printf("%s: shared mem addr = %d\n", __FILE__, host_dst);
  memcpy( dest, host_dst, src_size);
  // printf("\n%s: GOT resp from device, dest=%d", __FILE__, &dest);

  return 1;
}

void device_stop()
{
  message_queue req_q(open_only, g_req_msg_q_name);
  IPC_Request req;
  req.type = IPC_Request_Type_Stop;
  // send request
  req_q.send(&req, sizeof(req), 0);
}

int device_go()
{
  message_queue req_q(open_only, g_req_msg_q_name);
  message_queue resp_q(open_only, g_resp_msg_q_name);
  IPC_Request req;
  unsigned int priority = 0;
  message_queue::size_type recvd_size;

  req.type = IPC_Request_Type_Go;
  // send request
  req_q.send(&req, sizeof(req), 0);
  // receive response
  IPC_Response resp;
  resp_q.receive(&resp, sizeof(resp), recvd_size, priority);
  return resp.payloads[0];
}

/**
 * Have the host receive message that the device is done
 */
int host_recv()
{
  message_queue req_q(open_only, g_req_msg_q_name);
  message_queue resp_q(open_only, g_resp_msg_q_name);
  shared_memory_object shm (open_only, g_shared_mem_name, read_write);
  mapped_region region(shm, read_write);
  message_queue::size_type recvd_size;
  unsigned int priority = 0;

  IPC_Request req;
  req_q.receive(&req, sizeof(req), recvd_size, priority);

  switch(req.type) {
    case IPC_Request_Type_MemcpyDeviceToHost:
    {
      int dev_ptr = req.payloads[0];
      size_t size = req.payloads[1];
      // int* shm_ptr = static_cast<int*>(region.get_address());
      printf("\n%s: receive ack from device: %d, %d\n", 
      __FILE__, dev_ptr, size);
      return 1;
      break;
    }
    default:
    {
      return 0;
      break;
    }
  }
  return 0;
}