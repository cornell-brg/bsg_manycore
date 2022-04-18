#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

extern "C" __attribute__ ((noinline))
int kernel_appl_reduce(int *A, int *B, int *C, int size, int grain_size, int* dram_buffer) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(size);
    bsg_print_int(grain_size);
  }

  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, grain_size);

  // sync
  barrier.sync();

  if (__bsg_id == 0) {

    C[0] = appl::parallel_reduce(0, size, 0,
        [&](int start, int end, int initV) {
          int psum = initV;
          for (int i = start; i < end; i++) {
            psum += A[i];
          }
          return psum;
        },
        [](int x, int y) { return x + y; }
    );

  } else {
    appl::worker_thread_init();
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  barrier.sync();
  return 0;
}
