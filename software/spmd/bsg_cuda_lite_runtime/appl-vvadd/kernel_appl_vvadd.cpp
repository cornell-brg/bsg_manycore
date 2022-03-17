#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

__attribute__ ((noinline))
void vvadd_appl_pfor( int dest[], int src0[],
                      int src1[], int size ) {
  appl::parallel_for( 0, size,
      [&]( int i ) {
        dest[i] = src0[i] + src1[i];
      }
  );
}

extern "C" __attribute__ ((noinline))
int kernel_appl_vvadd(int *A, int *B, int *C, int size, int grain_size) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(size);
    bsg_print_int(grain_size);
  }

  // --------------------- kernel ------------------------
  appl::runtime_init(grain_size);

  // sync
  barrier.sync();

  if (__bsg_id == 0) {
    vvadd_appl_pfor(C, A, B, size);
  } else {
    appl::work_stealing_loop([&]() -> bool {
        return bsg_amoadd(&appl::global::g_stop_flag, 0);
        } );
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  barrier.sync();
  return 0;
}
