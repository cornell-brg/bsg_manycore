#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include<sys/wait.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "ipc.h"
#include "host_lib.h"

// Control bits for the processor/DRAM;

using namespace boost::interprocess;

int main(int argc, char* argv[])
{
  void* dev_dest;
  void* dev_src0;
  void* dev_src1;

  int src0[8] = {7, 7, 5, 8, 2, 5, 8, 0x104599};
  int src1[8] = {5,6,7,8,9,0,1,2};
  int dest[8];
  // forking and running the device from the host
  int pid;
  int status = 0;
  pid = fork();
  if (pid == 0) 
  { //child process
    char * argv_list[] = {"simv", "+vcs+stop+3000000",
    "-cm_dir $(PROG_NAME).vdb/ -cm line+tgl"};
    printf("%s: child process, pid = %u\n",__FILE__,getpid());
    execv("./simv", argv_list);
    exit(0);
  }
  else
  { 
    sleep(2);
    printf("%s: parent process, pid = %u\n",__FILE__,getppid());
    device_malloc(&dev_src0, 8);
    printf("%s: dev_src0 = %x\n", __FILE__, dev_src0);
    // device_malloc(&dev_src1, 8);
    // printf("%s: dev_src1 = %x\n", __FILE__, dev_src1);
    device_malloc(&dev_dest, 8);
    printf("%s: dev_dest = %x\n", __FILE__, dev_dest);
    
    printf("%s: sending memcpy to device\n", __FILE__);
    memcpy_host_to_device(dev_src0, src0, 8*sizeof(int));
    // printf("%s: sending memcpy to device\n", __FILE__);
    // memcpy_host_to_device(dev_src1, src1, 8*sizeof(int));

    device_go();
    printf("%s: Waiting for msg from device\n", __FILE__);
    (!(host_recv()));

    printf("%s: Retrieving mem from device addr = %#x\n", __FILE__, dev_src0);
    memcpy_device_to_host(dev_src0, dest, 8);
    printf("\n%s: Printing dev results: \n\n", __FILE__);
    for (int i = 0; i < 8; i++ ){
      printf("%#x\n", dest[i]);
    }
    printf("\n\n");

    // sleep(3);
    printf("%s: sending stop to device\n", __FILE__);
    device_stop();
    wait(&status);
  }

  return 0;
}

    // printf("%s: parent process, pid = %u\n",__FILE__,getppid()); 
    // printf("%s: sending malloc src0 to device\n", __FILE__);
    // device_malloc(&dev_src0, 16);
    // printf("%s: dev_src0 = %x\n", __FILE__, dev_src0);
    // printf("%s: sending malloc src1 to device\n", __FILE__);
    // device_malloc(&dev_src1, 8);
    // printf("%s: dev_src1 = %x\n", __FILE__, dev_src1);

    // printf("%s: sending src0 memcpy to device\n", __FILE__);
    // memcpy_host_to_device(dev_src0, src0, 8*sizeof(int));
    // printf("%s: sending src1 memcpy to device\n", __FILE__);
    // memcpy_host_to_device(dev_src1, src1, 4*sizeof(int));