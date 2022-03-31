#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"
#include "ligra.h"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

int32_t fib_base(int32_t n) {
  if (n < 2)
    return n;
  else
    return fib_base(n-1) + fib_base(n-2);
}

int32_t fib(int32_t n, int32_t gsize = 2) {
  if (n <= gsize) {
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
int kernel_appl_bfs(int* results, symmetricVertex* V, int n, int m, int* dram_buffer) {

  // debug print
  if (__bsg_id == 0) {
    graph<symmetricVertex> G = graph<symmetricVertex>(V, n, m, nullptr);
    uint32_t idx = 0;
    for (uint32_t v = 0; v < G.n; v++) {
      for (uint32_t e = 0; e <  G.V[v].getOutDegree(); e++) {
        results[idx] = G.V[v].getOutNeighbor(e);
        idx++;
      }
    }
  }

  barrier.sync();
  return 0;
}
