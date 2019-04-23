
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#include "../brg_dense_gemm_xcel/gemm-64.dat"

int wait_on_accelerator(int x, int y) {
  return bsg_global_ptr(x, y, 0)[0];
}

int matC_64[4096] __attribute__ ( ( section(".dram") ) );

// Allocate three arrays in the scratchpad for 16x16 matrix blocking
int tmp_A[16][16], tmp_B[16][16], tmp_C[16][16];

#define max(x, y) (((x)>(y))?(x):(y))
#define min(x, y) (((x)<(y))?(x):(y))

int main()
{
  bsg_set_tile_x_y();

  // Only core x,y=0,0 prepares data, since we set tile_group_Y = 1
  if ((__bsg_x == 0) && (__bsg_y == 0)) {

    int bsize = 16;
    bsg_print_time();

    for (int N_begin=0; N_begin<N_64; N_begin+=bsize) {
      int N_end    = min(N_begin + bsize, N_64);
      int N_stride = N_end - N_begin;

      for (int P_begin=0; P_begin<P_64; P_begin+=bsize) {
        int P_end    = min(P_begin + bsize, P_64);
        int P_stride = P_end - P_begin;

        // Calculate one block in result C array

        // Bring a block of C array into scratchpad
        for (int i=0; i<N_stride; ++i) {
          int *tmp_ptr = tmp_C[i];
          int *C_ptr   = &(matC_64[(i+N_begin)*P_64+P_begin]);

          for (int j=0; j<P_stride; ++j)
            *(tmp_ptr++) = *(C_ptr++);
        }

        // Enumerate A/B blocks
        for (int M_begin=0; M_begin<M_64; M_begin+=bsize) {
          int M_end    = min(M_begin + bsize, M_64);
          int M_stride = M_end - M_begin;

          // Bring a block of A array into scratchpad
          for (int i=0; i<N_stride; ++i) {
            int *tmp_ptr = tmp_A[i];
            int *A_ptr   = &(matA_64[(i+N_begin)*M_64+M_begin]);

            for (int k=0; k<M_stride; ++k)
              *(tmp_ptr++) = *(A_ptr++);
          }
          // Bring a block of B array into scratchpad
          for (int k=0; k<M_stride; ++k) {
            int *tmp_ptr = tmp_B[k];
            int *B_ptr   = &(matB_64[(k+M_begin)*P_64+P_begin]);

            for (int j=0; j<P_stride; ++j)
              *(tmp_ptr++) = *(B_ptr++);
          }

          // Operate on the temporary block
          for (int i=0; i<N_stride; ++i)
          for (int j=0; j<P_stride; ++j) {
            int c = tmp_C[i][j];
            for (int k=0; k<M_stride; ++k)
              c += tmp_A[i][k] * tmp_B[k][j];
            tmp_C[i][j] = c;
          }
        }
        // Store a block of C array back to scratchpad
        for (int i=0; i<N_stride; ++i) {
          int *tmp_ptr = tmp_C[i];
          int *C_ptr   = &(matC_64[(i+N_begin)*P_64+P_begin]);

          for (int j=0; j<P_stride; ++j)
            *(C_ptr++) = *(tmp_ptr++);
        }
      }
    }
    bsg_print_time();

    for (int i=0; i<N_64*P_64; ++i)
    if (matC_64[i] != refC_64[i]) {
      bsg_printf("mat[%d][%d], %d != ref %d\n", i/P_64, i%P_64, matC_64[i], refC_64[i]);
      bsg_fail();
    }
    // bsg_printf(" [ passed ] \n");

    bsg_finish();
  }

  bsg_wait_while(1);
}

