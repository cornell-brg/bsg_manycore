#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 4
#endif

/* A block is a 2D array of floats. */
typedef float Block[BLOCK_SIZE][BLOCK_SIZE];
#define BLOCK(B,I,J) (B[I][J])

/* A matrix is a 1D array of blocks. */
typedef Block *Matrix;
#define MATRIX(M,I,J) ((M)[(I)*nBlocks+(J)])

/* Matrix size in blocks. */
static int nBlocks;

/****************************************************************************\
 * Element operations.
 \****************************************************************************/
/*
 * elem_daxmy - Compute y' = y - ax where a is a float and x and y are
 * vectors of floats.
 */
static void elem_daxmy(float a, float *x, float *y, int n)
{
  for (n--; n >= 0; n--)
    y[n] -= a * x[n];
}

/****************************************************************************\
 * Block operations.
 \****************************************************************************/

/*
 * block_lu - Factor block B.
 */
static void block_lu(Block B)
{
  int i, k;

  /* Factor block. */
  for (k = 0; k < BLOCK_SIZE; k++)
    for (i = k + 1; i < BLOCK_SIZE; i++) {
      BLOCK(B, i, k) /= BLOCK(B, k, k);
      elem_daxmy(BLOCK(B, i, k), &BLOCK(B, k, k + 1),
                 &BLOCK(B, i, k + 1), BLOCK_SIZE - k - 1);
    }
}

/*
 * block_lower_solve - Perform forward substitution to solve for B' in
 * LB' = B.
 */
static void block_lower_solve(Block B, Block L)
{
  int i, k;

  /* Perform forward substitution. */
  for (i = 1; i < BLOCK_SIZE; i++)
    for (k = 0; k < i; k++)
      elem_daxmy(BLOCK(L, i, k), &BLOCK(B, k, 0),
                 &BLOCK(B, i, 0), BLOCK_SIZE);
}

/*
 * block_upper_solve - Perform forward substitution to solve for B' in
 * B'U = B.
 */
static void block_upper_solve(Block B, Block U)
{
  int i, k;

  /* Perform forward substitution. */
  for (i = 0; i < BLOCK_SIZE; i++)
    for (k = 0; k < BLOCK_SIZE; k++) {
      BLOCK(B, i, k) /= BLOCK(U, k, k);
      elem_daxmy(BLOCK(B, i, k), &BLOCK(U, k, k + 1),
                 &BLOCK(B, i, k + 1), BLOCK_SIZE - k - 1);
    }
}

/*
 * block_schur - Compute Schur complement B' = B - AC.
 */
static void block_schur(Block B, Block A, Block C)
{
  int i, k;

  /* Compute Schur complement. */
  for (i = 0; i < BLOCK_SIZE; i++)
    for (k = 0; k < BLOCK_SIZE; k++)
      elem_daxmy(BLOCK(A, i, k), &BLOCK(C, k, 0),
                 &BLOCK(B, i, 0), BLOCK_SIZE);
}

/****************************************************************************\
 * Divide-and-conquer matrix LU decomposition.
 \****************************************************************************/

/*
 * schur - Compute M' = M - VW.
 */

void schur(Matrix M, Matrix V, Matrix W, int nb)
{
  Matrix M00, M01, M10, M11;
  Matrix V00, V01, V10, V11;
  Matrix W00, W01, W10, W11;
  int hnb;

  /* Check base case. */
  if (nb == 1) {
    block_schur(*M, *V, *W);
    return;
  }
  /* Break matrices into 4 pieces. */
  hnb = nb / 2;
  M00 = &MATRIX(M, 0, 0);
  M01 = &MATRIX(M, 0, hnb);
  M10 = &MATRIX(M, hnb, 0);
  M11 = &MATRIX(M, hnb, hnb);
  V00 = &MATRIX(V, 0, 0);
  V01 = &MATRIX(V, 0, hnb);
  V10 = &MATRIX(V, hnb, 0);
  V11 = &MATRIX(V, hnb, hnb);
  W00 = &MATRIX(W, 0, 0);
  W01 = &MATRIX(W, 0, hnb);
  W10 = &MATRIX(W, hnb, 0);
  W11 = &MATRIX(W, hnb, hnb);

  /* Form Schur complement with recursive calls. */
  appl::parallel_invoke(
    [&] { schur(M00, V00, W00, hnb); },
    [&] { schur(M01, V00, W01, hnb); },
    [&] { schur(M10, V10, W00, hnb); },
    [&] { schur(M11, V10, W01, hnb); }
    );

  appl::parallel_invoke(
    [&] { schur(M00, V01, W10, hnb); },
    [&] { schur(M01, V01, W11, hnb); },
    [&] { schur(M10, V11, W10, hnb); },
    [&] { schur(M11, V11, W11, hnb); }
    );

  return;
}

/*
 * lower_solve - Compute M' where LM' = M.
 */

void lower_solve(Matrix M, Matrix L, int nb);

void aux_lower_solve(Matrix Ma, Matrix Mb, Matrix L, int nb)
{
  Matrix L00, L01, L10, L11;

  /* Break L matrix into 4 pieces. */
  L00 = &MATRIX(L, 0, 0);
  L01 = &MATRIX(L, 0, nb);
  L10 = &MATRIX(L, nb, 0);
  L11 = &MATRIX(L, nb, nb);

  /* Solve with recursive calls. */
  lower_solve(Ma, L00, nb);

  schur(Mb, L10, Ma, nb);

  lower_solve(Mb, L11, nb);
}

void lower_solve(Matrix M, Matrix L, int nb)
{
  Matrix M00, M01, M10, M11;
  int hnb;

  /* Check base case. */
  if (nb == 1) {
    block_lower_solve(*M, *L);
    return;
  }
  /* Break matrices into 4 pieces. */
  hnb = nb / 2;
  M00 = &MATRIX(M, 0, 0);
  M01 = &MATRIX(M, 0, hnb);
  M10 = &MATRIX(M, hnb, 0);
  M11 = &MATRIX(M, hnb, hnb);

  /* Solve with recursive calls. */
  appl::parallel_invoke(
    [&] { aux_lower_solve(M00, M10, L, hnb); },
    [&] { aux_lower_solve(M01, M11, L, hnb); }
    );

  return;
}

/*
 * upper_solve - Compute M' where M'U = M.
 */

void upper_solve(Matrix M, Matrix U, int nb);

void aux_upper_solve(Matrix Ma, Matrix Mb, Matrix U, int nb)
{
  Matrix U00, U01, U10, U11;

  /* Break U matrix into 4 pieces. */
  U00 = &MATRIX(U, 0, 0);
  U01 = &MATRIX(U, 0, nb);
  U10 = &MATRIX(U, nb, 0);
  U11 = &MATRIX(U, nb, nb);

  /* Solve with recursive calls. */
  upper_solve(Ma, U00, nb);

  schur(Mb, Ma, U01, nb);

  upper_solve(Mb, U11, nb);

  return;
}

void upper_solve(Matrix M, Matrix U, int nb)
{
  Matrix M00, M01, M10, M11;
  int hnb;

  /* Check base case. */
  if (nb == 1) {
    block_upper_solve(*M, *U);
    return;
  }
  /* Break matrices into 4 pieces. */
  hnb = nb / 2;
  M00 = &MATRIX(M, 0, 0);
  M01 = &MATRIX(M, 0, hnb);
  M10 = &MATRIX(M, hnb, 0);
  M11 = &MATRIX(M, hnb, hnb);

  /* Solve with recursive calls. */
  appl::parallel_invoke(
    [&] { aux_upper_solve(M00, M01, U, hnb); },
    [&] { aux_upper_solve(M10, M11, U, hnb); }
    );

  return;
}

/*
 * lu - Perform LU decomposition of matrix M.
 */

void lu(Matrix M, int nb)
{
  Matrix M00, M01, M10, M11;
  int hnb;

  /* Check base case. */
  if (nb == 1) {
    block_lu(*M);
    return;
  }
  /* Break matrix into 4 pieces. */
  hnb = nb / 2;
  M00 = &MATRIX(M, 0, 0);
  M01 = &MATRIX(M, 0, hnb);
  M10 = &MATRIX(M, hnb, 0);
  M11 = &MATRIX(M, hnb, hnb);

  /* Decompose upper left. */
  lu(M00, hnb);

  /* Solve for upper right and lower left. */
  appl::parallel_invoke(
    [&] { lower_solve(M01, M00, hnb); },
    [&] { upper_solve(M10, M00, hnb); }
    );

  /* Compute Schur complement of lower right. */
  schur(M11, M10, M01, hnb);

  /* Decompose lower right. */
  lu(M11, hnb);

  return;
}

extern "C" __attribute__ ((noinline))
int kernel_appl_lu(Matrix M, int n, int* dram_buffer) {

  nBlocks = n / BLOCK_SIZE;

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(n);
    bsg_print_int(nBlocks);
  }

  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, 1);

  // sync
  barrier.sync();

  if (__bsg_id == 0) {
    lu(M, nBlocks);
  } else {
    appl::worker_thread_init();
  }
  appl::runtime_end();
  // --------------------- end of kernel -----------------

  barrier.sync();
  return 0;
}
