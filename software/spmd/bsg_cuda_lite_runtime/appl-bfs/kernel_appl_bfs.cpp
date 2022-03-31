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
    bsg_print_int(n);
    bsg_print_int(m);
    graph<symmetricVertex> G = graph<symmetricVertex>(V, n, m, nullptr);
    bsg_print_int(G.V[1].getOutDegree());
    for (int i = 0; i < G.V[1].getOutDegree(); i++) {
      bsg_print_int(G.V[1].getInNeighbor(i));
    }
    bsg_print_int(sizeof(symmetricVertex));
    bsg_print_int(11111);
  }

  barrier.sync();
  return 0;
}
