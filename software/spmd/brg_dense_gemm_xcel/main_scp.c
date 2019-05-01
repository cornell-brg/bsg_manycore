#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#include "bsg_mutex.h"
#include "bsg_barrier.h"

#include "gemm-64.dat"
#include "gemm-128.dat"

int wait_on_accelerator(int x, int y) {
  return bsg_global_ptr(x, y, 0)[0];
}

// Currently we support up to 128x128 in this file
int matC[16384] __attribute__ ( ( section(".dram") ) );

// Barrier to sync cores at the end of execution
bsg_barrier tile0_barrier = BSG_BARRIER_INIT(0, bsg_tiles_X-1, 0, bsg_tiles_Y-1);

// declare local scratchpad here
int *scp_B[4*128];

int main()
{
  bsg_set_tile_x_y();

  if (bsg_global_X == 5 && bsg_global_Y == 7) { // 4x4 systolic
    int N, M, P, *matA, *matB, *refC;

    // Choose dataset based on -DBRG_DATASET=xxx

    if (BRG_DATASET == 64) {
      N = N_64; M = M_64; P = P_64;
      matA = matA_64; matB = matB_64; refC = refC_64;
    } else if (BRG_DATASET == 128) {
      N = N_128; M = M_128; P = P_128;
      matA = matA_128; matB = matB_128; refC = refC_128;
    } else {
      bsg_printf("Dataset <%d> doesn't exist!\n", BRG_DATASET);
      bsg_fail();
    }

    if (__bsg_y == 0) {

      int C_stride = 4, tile_dim = 4;
      int total_C_stride = C_stride*tile_dim;
      int ceil_A_rows = (N-1) / tile_dim + 1;

      // Every core needs to launch a number of times

      if (__bsg_x == 0) {
        bsg_print_time();
        bsg_remote_int_ptr rE1_base = bsg_global_ptr(0, 3, 0);
        bsg_remote_int_ptr rE2_base = bsg_global_ptr(0, 4, 0);
        bsg_remote_int_ptr rE3_base = bsg_global_ptr(0, 5, 0);
        bsg_remote_int_ptr rE4_base = bsg_global_ptr(0, 6, 0);

        // the first core configures row engine

        int engine_A_elements = ceil_A_rows * M;

        for (int i=0; i<P; i+=total_C_stride){
          int *A_ptr = matA;
          rE1_base[1] = A_ptr; A_ptr += engine_A_elements;
          rE2_base[1] = A_ptr; A_ptr += engine_A_elements;
          rE3_base[1] = A_ptr; A_ptr += engine_A_elements;
          rE4_base[1] = A_ptr;

          rE1_base[2] = rE2_base[2] = rE3_base[2] = rE4_base[2] = engine_A_elements;
          rE1_base[3] = rE2_base[3] = rE3_base[3] = rE4_base[3] = M;
          rE1_base[0] = rE2_base[0] = rE3_base[0] = rE4_base[0] = 1;

          for (int y=0; y<4; ++y)
            wait_on_accelerator(0, 3+y);
        }
        bsg_barrier_wait(&tile0_barrier, 0, 0);

        bsg_print_time();

        for (int i=0; i<N*P; ++i)
        if (matC[i] != refC[i]) {
          bsg_printf("mat[%d][%d], %d != ref %d\n", i/P, i%P, matC[i], refC[i]);
          bsg_fail();
        }
        bsg_printf(" [ passed ] \n");
        bsg_finish();
      }
      else {
        // the rest of the cores configure col and xcel

        bsg_remote_int_ptr my_cE_base = bsg_global_ptr(__bsg_x, 2, 0);

        int *B_ptr = matB;
        B_ptr += (__bsg_x - 1) * C_stride;

        int round_elements = M * C_stride;
        int skip_entries = (P - C_stride + 1);
        int total_elements = round_elements * ceil_A_rows;

        for (int i=0; i<P; i+=total_C_stride) {
          int *B_ptr_tmp   = B_ptr;
          int *scp_ptr_tmp = scp_B;

          bsg_print_time();
          // Bring a 4xM stride of B array into scratchpad
          for (int k=0; k<M; ++k) {
            *(scp_ptr_tmp++) = *(B_ptr_tmp++);
            *(scp_ptr_tmp++) = *(B_ptr_tmp++);
            *(scp_ptr_tmp++) = *(B_ptr_tmp++);
            *(scp_ptr_tmp++) = *B_ptr_tmp;

            // skip to the next stride
            B_ptr_tmp += skip_entries;
          }
          B_ptr += total_C_stride;
          bsg_print_time();

          // configure col engine
          // Give the col engine a remote address of this tile
          my_cE_base[1] = bsg_remote_ptr(__bsg_x, __bsg_y, scp_B);
          my_cE_base[2] = round_elements;
          my_cE_base[3] = C_stride;
          my_cE_base[4] = 4;//skip_addr;
          my_cE_base[5] = total_elements;
          my_cE_base[0] = 1;

          for (int y=0; y<tile_dim; ++y) {
            bsg_remote_int_ptr base_ptr = bsg_global_ptr(__bsg_x, 3+y, 0);
            base_ptr[1] = ceil_A_rows;
            base_ptr[2] = 0;
            base_ptr[4] = P * 4;
            base_ptr[5] = &(matC[y*ceil_A_rows*P + i + C_stride*(__bsg_x-1)]);
            base_ptr[6] = C_stride;
            base_ptr[0] = 1;
          }
          bsg_print_time();

          wait_on_accelerator(__bsg_x, 2);
          for (int y=0; y<4; ++y)
            wait_on_accelerator(__bsg_x, 3+y);
          /*bsg_print_time();*/
        }
        bsg_barrier_wait( &tile0_barrier, 0, 0);
      }
    }
  }
  bsg_wait_while(1);
}
