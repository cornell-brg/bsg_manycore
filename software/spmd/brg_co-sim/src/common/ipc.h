//========================================================================
// ipc.h
//========================================================================

#include <stdint.h>

//------------------------------------------------------------------------
// IPC request and response
//------------------------------------------------------------------------

enum IPC_Request_Type {
  IPC_Request_Type_Malloc,
  IPC_Request_Type_MemcpyDeviceToHost,
  IPC_Request_Type_MemcpyHostToDevice,
  IPC_Request_Type_Stop,
  IPC_Request_Type_Go
};

struct IPC_Request {
  IPC_Request_Type type;
  uint64_t payloads[4];
};

struct IPC_Response {
  uint64_t payloads[4];
};

//------------------------------------------------------------------------
// IPC queues
//------------------------------------------------------------------------

static const char* g_req_msg_q_name  = "REQ_MSG_QUEUE";
static const char* g_resp_msg_q_name = "RESP_MSG_QUEUE";

//------------------------------------------------------------------------
// Shared memory section
//------------------------------------------------------------------------

static const char* g_shared_mem_name = "SHARED_MEM_SECTION";
static const int   g_shared_mem_size = 1024;

