#include <iostream>
#include <stdio.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "ipc.h"
#include "host_lib.h"

using namespace boost::interprocess;

int main(int argc, char* argv[])
{
  void* dev_ptr;
  int host_ptr[8] = {1, 7, 5, 8, 2, 5, 8, 0x99};

  printf("%s: sending malloc to device\n", __FILE__);
  device_malloc(&dev_ptr, 16);
  printf("%s: dev_ptr = %x\n", __FILE__, dev_ptr);

  printf("%s: sending memcpy to device\n", __FILE__);
  memcpy_host_to_device(dev_ptr, host_ptr, 8*sizeof(int));

  printf("%s: sending stop to device\n", __FILE__);
  device_stop();

  return 0;
}