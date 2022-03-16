#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

int32_t fib_base(int32_t n) {
  if (n < 2)
    return n;
  else
    return fib_base(n-1) + fib_base(n-2);
}

int32_t fib(int32_t n, int32_t gsize = 2) {
  if (n < gsize) {
    return fib_base(n);
  }

  int32_t x, y;

  appl::parallel_invoke(
      [&] { x = fib(n-1, gsize); },
      [&] { y = fib(n-2, gsize); }
      );

  return x + y;
}

void outter_loop(int32_t dest[], int32_t n, int32_t gsize) {
  appl::parallel_for( 0, 4,
      [&]( int i ) {
        dest[i] = fib( (n-i), gsize );
      }
  );
}

extern "C" __attribute__ ((noinline))
int kernel_appl_parallel_fib(int32_t* dest, int n, int grain_size) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(n);
    bsg_print_int(grain_size);
  }

  // sync
  barrier.sync();

  // --------------------- kernel ------------------------
  appl::runtime_init(1);
  if (__bsg_id == 0) {
    outter_loop(dest, n, grain_size);
  } else {
    appl::work_stealing_loop([&]() -> bool {
        return bsg_amoadd(&appl::global::g_stop_flag, 0);
        } );
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  if (__bsg_id == 0) {
    for (int i = 0; i < 4; i++) {
      bsg_print_int(dest[i]);
    }
  }

  barrier.sync();
  return 0;
}
