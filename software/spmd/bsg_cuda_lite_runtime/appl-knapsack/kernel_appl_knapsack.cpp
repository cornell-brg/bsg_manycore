#include <limits.h>
#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

struct item {
  int value;
  int weight;
};

int best_so_far __attribute__ ((section (".dram"))) = INT_MIN;

/*
 * return the optimal solution for n items (first is e) and
 * capacity c. Value so far is v.
 */
int knapsack( struct item* e, int c, int n, int v )
{
  int    with, without, best;
  float ub;

  /* base case: full knapsack or no items */
  if ( c < 0 )
    return INT_MIN;

  if ( n == 0 || c == 0 )
    return v; /* feasible solution, with value v */

  ub = (float)v + c * e->value / e->weight;

  if ( ub < best_so_far ) {
    /* prune ! */
    return INT_MIN;
  }

  appl::parallel_invoke(
      [&] {
        /*
         * compute the best solution without the current item in the
         * knapsack
         */
        without = knapsack( e + 1, c, n - 1, v );
      },
      [&] {
        /* compute the best solution with the current item in the knapsack
         */
        with = knapsack( e + 1, c - e->weight, n - 1, v + e->value );
      } );

  best = with > without ? with : without;

  /*
   * notice the race condition here. The program is still
   * correct, in the sense that the best solution so far
   * is at least best_so_far. Moreover best_so_far gets updated
   * when returning, so eventually it should get the right
   * value. The program is highly non-deterministic.
   */
  // it would be nice to have amo_max
  if ( best > best_so_far )
    best_so_far = best;

  return best;
}

extern "C" __attribute__ ((noinline))
int kernel_appl_knapsack(int *results, struct item* items, int n, int capacity, int* dram_buffer) {

  int sol;

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(n);
    bsg_print_int(capacity);
  }

  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, 1);

  // sync
  barrier.sync();

  if (__bsg_id == 0) {
    sol = knapsack( items, capacity, n, 0 );
    results[0] = sol;
    bsg_print_int(sol);
  } else {
    appl::worker_thread_init();
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  barrier.sync();
  return 0;
}
