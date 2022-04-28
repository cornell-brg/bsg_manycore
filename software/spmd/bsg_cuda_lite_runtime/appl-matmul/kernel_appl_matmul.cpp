#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

#define REAL float
int grain_size;

inline void copy16(float* src, float* dst) {
  register float tmp0 = src[0];
  register float tmp1 = src[1];
  register float tmp2 = src[2];
  register float tmp3 = src[3];
  register float tmp4 = src[4];
  register float tmp5 = src[5];
  register float tmp6 = src[6];
  register float tmp7 = src[7];
  register float tmp10 = src[8];
  register float tmp11 = src[9];
  register float tmp12 = src[10];
  register float tmp13 = src[11];
  register float tmp14 = src[12];
  register float tmp15 = src[13];
  register float tmp16 = src[14];
  register float tmp17 = src[15];
  asm volatile("": : :"memory");
  dst[0] = tmp0;
  dst[1] = tmp1;
  dst[2] = tmp2;
  dst[3] = tmp3;
  dst[4] = tmp4;
  dst[5] = tmp5;
  dst[6] = tmp6;
  dst[7] = tmp7;
  dst[8] = tmp10;
  dst[9] = tmp11;
  dst[10] = tmp12;
  dst[11] = tmp13;
  dst[12] = tmp14;
  dst[13] = tmp15;
  dst[14] = tmp16;
  dst[15] = tmp17;
}

/*
 * A \in M(m, n)
 * B \in M(n, p)
 * C \in M(m, p)
 */

inline void compute4x4(float* A, float* B, float* C, int m, int n, int p) {
  int i, j, k;
  for ( i = 0; i < m; i += 4 ) {
    for ( k = 0; k < p; k += 4 ) {
      int dest_base = i * p + k;
      register float res00 = C[dest_base + 0     + 0];
      register float res01 = C[dest_base + 0     + 1];
      register float res02 = C[dest_base + 0     + 2];
      register float res03 = C[dest_base + 0     + 3];
      register float res10 = C[dest_base + p     + 0];
      register float res11 = C[dest_base + p     + 1];
      register float res12 = C[dest_base + p     + 2];
      register float res13 = C[dest_base + p     + 3];
      register float res20 = C[dest_base + 2 * p + 0];
      register float res21 = C[dest_base + 2 * p + 1];
      register float res22 = C[dest_base + 2 * p + 2];
      register float res23 = C[dest_base + 2 * p + 3];
      register float res30 = C[dest_base + 3 * p + 0];
      register float res31 = C[dest_base + 3 * p + 1];
      register float res32 = C[dest_base + 3 * p + 2];
      register float res33 = C[dest_base + 3 * p + 3];
      for ( j = 0; j < n; j++ ) {
        int mat1_base = i * n + j;
        register float mat1_0 = A[mat1_base + 0 * n];
        register float mat1_1 = A[mat1_base + 1 * n];
        register float mat1_2 = A[mat1_base + 2 * n];
        register float mat1_3 = A[mat1_base + 3 * n];
        int mat2_base = j * p + k;
        register float mat2_0 = B[mat2_base + 0];
        register float mat2_1 = B[mat2_base + 1];
        register float mat2_2 = B[mat2_base + 2];
        register float mat2_3 = B[mat2_base + 3];
        res00 += mat1_0 * mat2_0;
        res01 += mat1_0 * mat2_1;
        res02 += mat1_0 * mat2_2;
        res03 += mat1_0 * mat2_3;
        res10 += mat1_1 * mat2_0;
        res11 += mat1_1 * mat2_1;
        res12 += mat1_1 * mat2_2;
        res13 += mat1_1 * mat2_3;
        res20 += mat1_2 * mat2_0;
        res21 += mat1_2 * mat2_1;
        res22 += mat1_2 * mat2_2;
        res23 += mat1_2 * mat2_3;
        res30 += mat1_3 * mat2_0;
        res31 += mat1_3 * mat2_1;
        res32 += mat1_3 * mat2_2;
        res33 += mat1_3 * mat2_3;
      }
      C[dest_base + 0     + 0] = res00;
      C[dest_base + 0     + 1] = res01;
      C[dest_base + 0     + 2] = res02;
      C[dest_base + 0     + 3] = res03;
      C[dest_base + p     + 0] = res10;
      C[dest_base + p     + 1] = res11;
      C[dest_base + p     + 2] = res12;
      C[dest_base + p     + 3] = res13;
      C[dest_base + 2 * p + 0] = res20;
      C[dest_base + 2 * p + 1] = res21;
      C[dest_base + 2 * p + 2] = res22;
      C[dest_base + 2 * p + 3] = res23;
      C[dest_base + 3 * p + 0] = res30;
      C[dest_base + 3 * p + 1] = res31;
      C[dest_base + 3 * p + 2] = res32;
      C[dest_base + 3 * p + 3] = res33;
    }
  }
}

void rec_matmul( REAL* A, REAL* B, REAL* C, int m, int n, int p, int ld,
                 bool add )
{
  int i, j, k;
  /* base case */
  REAL spmA[m*16];
  REAL spmB[16*p];
  REAL spmC[m*p];
  REAL* ptrC = &(spmC[0]);

  for ( i = 0; i < m; i++ ) {
    for ( k = 0; k < p; k += 8 ) {
      ptrC[0] = 0.0f;
      ptrC[1] = 0.0f;
      ptrC[2] = 0.0f;
      ptrC[3] = 0.0f;
      ptrC[4] = 0.0f;
      ptrC[5] = 0.0f;
      ptrC[6] = 0.0f;
      ptrC[7] = 0.0f;
      ptrC += 8;
    }
  }

  for ( int nn = 0; nn < n; nn += 16 ) {
    REAL* ptrA = &(spmA[0]);
    REAL* ptrB = &(spmB[0]);
    for ( i = 0; i < m; i++ ) {
        copy16(&(A[i * ld + nn]), ptrA);
        ptrA += 16;
    }
   for ( j = 0; j < 16; j++ ) {
       copy16(&(B[nn * ld + j * ld]), ptrB);
       ptrB += 16;
    }

    // compute
    compute4x4(spmA, spmB, spmC, m, 16, p);
  }

  ptrC = &(spmC[0]);
  for ( i = 0; i < m; i++ ) {
    for ( k = 0; k < p; k += 16 ) {
      copy16(ptrC, &(C[i * ld + k]));
      ptrC += 16;
    }
  }
}

extern "C" __attribute__ ((noinline))
int kernel_appl_matmul(REAL* A, REAL* B, REAL* C, int n, int _grain_size, int* dram_buffer) {

  // set global grain_size variable
  grain_size = _grain_size;

  int num_blk_per_axis = (n + 16 - 1) / 16;
  int num_output_blk = num_blk_per_axis * num_blk_per_axis;

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(n);
    bsg_print_int(grain_size);
  }

  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, grain_size);

  // sync
  barrier.sync();

  bsg_cuda_print_stat_kernel_start();
  if (__bsg_id == 0) {
    appl::parallel_for_1(0, num_output_blk,
      [&](int idx) {
        int blk_x = idx % num_blk_per_axis;
        int blk_y = idx / num_blk_per_axis;
        rec_matmul( A + blk_y * 16 * n, B + blk_x * 16,
            C + blk_y * 16 * n + blk_x * 16, 16, n, 16, n, false );
      }
    );
  } else {
    appl::worker_thread_init();
  }
  appl::runtime_end();
  bsg_cuda_print_stat_kernel_end();
  // --------------------- end of kernel -----------------

  barrier.sync();
  return 0;
}
