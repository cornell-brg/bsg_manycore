#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

__attribute__ ((noinline))
void vvadd_appl_pfor( int dest[], int src0[],
                      int src1[], int size ) {
  appl::parallel_for( 0, size,
      [dest, src0, src1]( int i ) {
        dest[i] = src0[i] + src1[i];
      }
  );
}

extern "C" __attribute__ ((noinline))
int kernel_appl_vvadd(int *A, int *B, int *C, int size, int grain_size, int* dram_buffer) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(size);
    bsg_print_int(grain_size);
  }

  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, grain_size);

  // sync
  appl::sync();
  bsg_cuda_print_stat_kernel_start();

  if (__bsg_id == 0) {
    vvadd_appl_pfor(C, A, B, size);
  } else {
    appl::worker_thread_init();
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  bsg_cuda_print_stat_kernel_end();
  appl::sync();
  return 0;
}
