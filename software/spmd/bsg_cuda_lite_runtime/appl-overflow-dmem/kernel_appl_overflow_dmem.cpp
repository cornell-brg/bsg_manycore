#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"
#define N 1536

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

__attribute__ ((noinline))
void vvadd_appl_pfor( int dest[], int src0[],
                      int src1[], int size ) {
  for (int i = 0; i < size; i++) {
    dest[i] = src0[i] + src1[i];
  }
}

extern "C" __attribute__ ((noinline))
int kernel_appl_overflow_dmem(int *A, int *B, int *C, int size, int grain_size) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(size);
    bsg_print_int(grain_size);
  }

  // sync
  barrier.sync();

  int buf[N];

  // --------------------- kernel ------------------------
  appl::runtime_init(grain_size);
  if (__bsg_id == 0) {
    vvadd_appl_pfor(buf, A, B, size);
  }
  barrier.sync();
  if (__bsg_id == 1) {
    int* remote_buf = appl::remote_ptr(buf, 0);
    vvadd_appl_pfor(C, A, remote_buf, size);
  }
  barrier.sync();
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  barrier.sync();
  return 0;
}
