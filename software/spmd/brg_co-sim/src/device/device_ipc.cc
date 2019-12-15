//========================================================================
// device_ipc.cc
//========================================================================
// Interprocess module for device

#include <iostream>
#include <stdio.h>

#include "svdpi.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "ipc.h"

using namespace boost::interprocess;

#ifdef __cplusplus
extern "C" {
#endif

#define STATE_MEMCPY 2
#define STATE_GO     3
#define STATE_MALLOC 4
#define STATE_STOP   5 
#define STATE_DEV_HOST   8 

//------------------------------------------------------------------------
// verilog funtions
//------------------------------------------------------------------------

extern void write_mem(int, int);
extern void read_mem_req(int, int);
extern int cosim_malloc(int);
//------------------------------------------------------------------------
// Globals
//------------------------------------------------------------------------

struct ShmRemover
{
  void remove()  {
    shared_memory_object::remove(g_shared_mem_name);
    message_queue::remove(g_req_msg_q_name);
    message_queue::remove(g_resp_msg_q_name);
    std::cout << __FILE__ << ": Shm and msq removed" << std::endl;
  }

  ShmRemover()  { remove(); }
  ~ShmRemover() { remove(); }
} g_remover;

shared_memory_object g_shm(create_only, g_shared_mem_name, read_write);
message_queue        g_req_q(create_only, g_req_msg_q_name, 1, sizeof(IPC_Request));
message_queue        g_resp_q(create_only, g_resp_msg_q_name, 1, sizeof(IPC_Response));
mapped_region        g_region;

//------------------------------------------------------------------------
// shm_init
//------------------------------------------------------------------------

void shm_init()
{
  g_shm.truncate(g_shared_mem_size);
  g_region = mapped_region(g_shm, read_write);
}

//------------------------------------------------------------------------
// receive request
//------------------------------------------------------------------------

int receive_request()
{
  message_queue::size_type recvd_size;
  unsigned int priority = 0;

  IPC_Request req;
  g_req_q.receive(&req, sizeof(req), recvd_size, priority);

  switch(req.type) {
  // allocate device memory
  case IPC_Request_Type_Malloc:
  {
    size_t size = req.payloads[0];
    printf("%s: Malloc received, size = %lu\n", __FILE__, size);
    // FIXME: assuming device memory starts at zero
    void* addr = (void*) cosim_malloc(size);
    printf("%s: malloc address = %x\n", __FILE__, addr);

    // send response
    IPC_Response resp;
    resp.payloads[0] = (uint64_t)addr;
    g_resp_q.send(&resp, sizeof(resp), 0);
    return STATE_MALLOC;
    break;
  }

  // memcpy from device to host
  case IPC_Request_Type_MemcpyDeviceToHost:
  {
    int dev_ptr = req.payloads[0];
    size_t size = req.payloads[1];
    read_mem_req(dev_ptr, size);
    return STATE_DEV_HOST;
    break;
  }

  // memcpy from host to device
  case IPC_Request_Type_MemcpyHostToDevice:
  {
    int dev_ptr = req.payloads[0];
    size_t size = req.payloads[1];
    printf("%s: memcpy host to device received, dev_ptr = %x, size = %lu bytes\n",
           __FILE__, dev_ptr, size);
    int* shm_ptr = static_cast<int*>(g_region.get_address());
    
    for (int i = 0; i < size; i += sizeof(int)) {
      // call verilog function to write device memory
      printf("%s: writing %d-th value (word) = %d into device memory\n",
             __FILE__, i / sizeof(int), *shm_ptr);
      write_mem(dev_ptr, *shm_ptr);
      dev_ptr += sizeof(int);
      shm_ptr++;
    }
    // send response
    IPC_Response resp;
    resp.payloads[0] = 1;
    g_resp_q.send(&resp, sizeof(resp), 0);

    return STATE_MEMCPY;
    break;
  }
  case IPC_Request_Type_Go:
  {
    IPC_Response resp;
    resp.payloads[0] = 1;
    g_resp_q.send(&resp, sizeof(resp), 0);
    return STATE_GO;
    break;
  }
  case IPC_Request_Type_Stop:
  {
    return STATE_STOP;
    break;
  }
  default:
  {
    // do nothing
    return 0;
    break;
  }
  } // switch(req.type)
  return 0;
}

// //------------------------------------------------------------------------
// // Send Request
// //------------------------------------------------------------------------

int send_request( int data ) 
{
  size_t size = 1;

  memcpy(g_region.get_address(), &data, size);
  unsigned int priority = 1;

  message_queue::size_type recvd_size;
  
  // prepare request
  IPC_Request req;
  req.type = IPC_Request_Type_MemcpyDeviceToHost;
  req.payloads[0] = (uint64_t)data;
  req.payloads[1] = 1;

  // send request
  g_req_q.send(&req, sizeof(req), priority);

  // receive response
  // IPC_Response resp;
  // resp_q.receive(&resp, sizeof(resp), recvd_size, priority);
  return 1;//resp.payloads[0];
}

// //------------------------------------------------------------------------
// // Build response
// //------------------------------------------------------------------------
int temp[500];
int counter = 0;

void build_response( int data )
{

  temp[counter++] = data;
  // printf("%s: writing to temp[%d]=%d with %d\n", __FILE__, counter-1, temp[counter-1], data);
}

int send_response() 
{
  printf("%s: Sendin Rsp - memcpyDtoH\n", __FILE__);
  unsigned int priority = 0;

  void* address = g_region.get_address();
  memcpy(address, temp, counter);
  // for (int i = 0; i < counter; i++){
  //   resp.payloads[i+1] = temp[i];
  // }

  // send response
  IPC_Response resp;
  resp.payloads[0] = (uint64_t)address;
  resp.payloads[1] = (uint64_t)counter;
  g_resp_q.send(&resp, sizeof(resp), priority);
  
  // printf("%s: addr = %d\n", __FILE__, address);


  counter = 0;
  return 1;//resp.payloads[0];
}

#ifdef __cplusplus
}
#endif
