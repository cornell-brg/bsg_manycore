#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

#define VERIFY 1

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

int nsolutions __attribute__ ((section (".dram")));

/*
 * <a> contains array of <n> queen positions.  Returns 1
 * if none of the queens conflict, and returns 0 otherwise.
 */
int ok(int n, char *a) {
  int i, j;
  int p, q;

  for (i = 0; i < n; i++) {
    p = a[i];

    for (j = i + 1; j < n; j++) {
      q = a[j];
      if (q == p || q == p - (j - i) || q == p + (j - i))
        return 0;
    }
  }
  return 1;
}

/*
 * <a> is an array of <j> numbers.  The entries of <a> contain
 * queen positions already set.  If there is any extension of <a>
 * to a complete <n> queen setting, returns one of these queen
 * settings (allocated from the heap).  Otherwise, returns NULL.
 * Does not side-effect <a>.
 */
void nqueens(int n, int j, char *a) {

  if (n == j) {
    if ( VERIFY ) {
      bsg_amoadd(&nsolutions, 1);
    }
  }

  /* try each possible position for queen <j> */
  appl::parallel_for( 0, n, 1, [&]( int i ) {
      /* allocate a temporary array and copy <a> into it */
      // char* b = (char*)malloc((j + 1) * sizeof(char));
      char b[j+1];
      for ( int k = 0; k < j; k++ ) {
        b[k] = a[k];
      }
      b[j] = i;
      if ( ok(j + 1, b) ) {
        nqueens(n, j + 1, b);
      }
    } );
}

extern "C" __attribute__ ((noinline))
int kernel_appl_nqueens(int* results, int n, int grain_size, int* dram_buffer) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(n);
    bsg_print_int(grain_size);
  }

  // output
  int32_t result     = -1;

  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, grain_size);

  // sync
  barrier.sync();

  if (__bsg_id == 0) {
    char a[n];
    nqueens(n, 0, a);
    result     = bsg_amoadd(&nsolutions, 0);
    results[0] = result;
  } else {
    appl::worker_thread_init();
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  bsg_print_int(result);

  barrier.sync();
  return 0;
}
