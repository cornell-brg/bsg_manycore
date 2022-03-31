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
int kernel_appl_bfs(int* results, int n, int grain_size, int* dram_buffer) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(n);
    bsg_print_int(grain_size);
    uintE neighbors[5];
    neighbors[0] = 0;
    neighbors[1] = 1;
    neighbors[2] = 2;
    neighbors[3] = 3;
    neighbors[4] = 4;
    symmetricVertex v = symmetricVertex(neighbors, 5);
    graph<symmetricVertex> G = graph<symmetricVertex>(&v, 1, 5, nullptr);
    bsg_print_int(G.V[0].getOutDegree());
    bsg_print_int(G.V[0].getInNeighbor(2));
  }

  // output
  int32_t result     = -1;

  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, 2);

  // sync
  barrier.sync();
  bsg_cuda_print_stat_kernel_start();

  if (__bsg_id == 0) {
    result = fib(n, grain_size);
    results[0] = result;
  } else {
    appl::work_stealing_loop([&]() -> bool {
        return bsg_amoadd(&appl::global::g_stop_flag, 0);
        } );
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  bsg_cuda_print_stat_kernel_end();
  bsg_print_int(result);

  barrier.sync();
  return 0;
}
