#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

#define REAL float

int g_base;

#define BASE_ROW  g_base
#define BASE_COL  g_base

void mat_transpose( REAL *A, REAL *B, const int size, int m, int n, int r_off, int col_off ) {
  int i, j;
  if (m < BASE_COL && n < BASE_ROW) {
    int r_max = r_off + n;
    int c_max = col_off + m;
    for ( i = r_off; i < r_max; i++ ) {
      for ( j = col_off; j < c_max; j++ ) {
        B[j * size + i] = A[i * size + j];
      }
    }
  } else {
    if (n >= m) {
      int split = n/2;
      appl::parallel_invoke(
        [&] { mat_transpose( A, B, size, m, split, r_off, col_off ); },
        [&] { mat_transpose( A, B, size, m, split, r_off, col_off + split ); }
        );
    }
    else {
      int split = m/2;
      appl::parallel_invoke(
        [&] { mat_transpose( A, B, size, split, n, r_off, col_off ); },
        [&] { mat_transpose( A, B, size, split, n, r_off + split, col_off ); }
        );
    }
  }
  return;
}

extern "C" __attribute__ ((noinline))
int kernel_appl_mattranspose(REAL *A, REAL *B, int size, int gbase) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(size);
    bsg_print_int(gbase);
  }

  // --------------------- kernel ------------------------
  appl::runtime_init(1);
  g_base = gbase;

  // sync
  barrier.sync();

  if (__bsg_id == 0) {
    mat_transpose(A, B, size, size, size, 0, 0);
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
