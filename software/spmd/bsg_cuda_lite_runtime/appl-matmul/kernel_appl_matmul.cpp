#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

#define REAL float
int grain_size;

void zero( REAL* A, int n )
{
  int i, j;

  for ( i = 0; i < n; i++ ) {
    for ( j = 0; j < n; j++ ) {
      A[i * n + j] = 0.0;
    }
  }
}

/*
 * A \in M(m, n)
 * B \in M(n, p)
 * C \in M(m, p)
 */

void rec_matmul( REAL* A, REAL* B, REAL* C, int m, int n, int p, int ld,
                 bool add )
{
  if ( ( m + n + p ) <= grain_size ) {
    int i, j, k;
    /* base case */
    if ( add ) {
      for ( i = 0; i < m; i++ )
        for ( k = 0; k < p; k++ ) {
          REAL c = 0.0;
          for ( j = 0; j < n; j++ )
            c += A[i * ld + j] * B[j * ld + k];
          C[i * ld + k] += c;
        }
    }
    else {
      for ( i = 0; i < m; i++ )
        for ( k = 0; k < p; k++ ) {
          REAL c = 0.0;
          for ( j = 0; j < n; j++ )
            c += A[i * ld + j] * B[j * ld + k];
          C[i * ld + k] = c;
        }
    }
  }
  else if ( m >= n && n >= p ) {
    int m1 = m >> 1;
    appl::parallel_invoke(
        [&] { rec_matmul( A, B, C, m1, n, p, ld, add ); },
        [&] {
          rec_matmul( A + m1 * ld, B, C + m1 * ld, m - m1, n, p, ld,
                      add );
        } );
  }
  else if ( n >= m && n >= p ) {
    int n1 = n >> 1;
    rec_matmul( A, B, C, m, n1, p, ld, add );
    rec_matmul( A + n1, B + n1 * ld, C, m, n - n1, p, ld, true );
  }
  else {
    int p1 = p >> 1;
    appl::parallel_invoke(
        [&] { rec_matmul( A, B, C, m, n, p1, ld, add ); },
        [&] { rec_matmul( A, B + p1, C + p1, m, n, p - p1, ld, add ); } );
  }
}

extern "C" __attribute__ ((noinline))
int kernel_appl_matmul(REAL* A, REAL* B, REAL* C, int n, int _grain_size, int* dram_buffer) {

  // set global grain_size variable
  grain_size = _grain_size;

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(n);
    bsg_print_int(grain_size);
  }

  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, grain_size);

  // sync
  barrier.sync();

  if (__bsg_id == 0) {
    rec_matmul( A, B, C, n, n, n, n, 0 );
  } else {
    appl::worker_thread_init();
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  barrier.sync();
  return 0;
}
