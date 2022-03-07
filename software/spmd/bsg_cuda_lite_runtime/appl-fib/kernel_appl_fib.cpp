#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.h"

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

extern "C" __attribute__ ((noinline))
int kernel_appl_fib(int n, int grain_size) {

  // sync
  barrier.sync();

  // output
  int32_t result     = -1;

  // --------------------- kernel ------------------------
  appl::runtime_init();
  result = fib(n, grain_size);
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  barrier.sync();
  return 0;
}
