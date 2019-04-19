
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#include "gemm-64.dat"

int wait_on_accelerator(int x, int y) {
  return bsg_global_ptr(x, y, 0)[0];
}

int matC_64[4096] __attribute__ ( ( section(".dram") ) );

int main()
{
  bsg_set_tile_x_y();

  // Only core x,y=0,0 prepares data, since we set tile_group_Y = 1
  if ((__bsg_x == 0) && (__bsg_y == 0)) {

    if (bsg_global_X == 5 && bsg_global_Y == 7) { // 4x4 systolic
      // do 64x64 matmul

      int N = N_64, M = M_64, P = P_64;

      int C_stride = 4, tile_dim = 4;
      int ceil_A_rows = (N-1) / tile_dim + 1;
      int engine_A_elements = ceil_A_rows * M;

      bsg_print_time();

      int *B_ptr = matB_64;

      bsg_remote_int_ptr rE1_base = bsg_global_ptr(0, 3, 0);
      bsg_remote_int_ptr rE2_base = bsg_global_ptr(0, 4, 0);
      bsg_remote_int_ptr rE3_base = bsg_global_ptr(0, 5, 0);
      bsg_remote_int_ptr rE4_base = bsg_global_ptr(0, 6, 0);

      bsg_remote_int_ptr cE1_base = bsg_global_ptr(1, 2, 0);
      bsg_remote_int_ptr cE2_base = bsg_global_ptr(2, 2, 0);
      bsg_remote_int_ptr cE3_base = bsg_global_ptr(3, 2, 0);
      bsg_remote_int_ptr cE4_base = bsg_global_ptr(4, 2, 0);

      // stripmining loop

      for (int i=0; i<P; i+=C_stride*tile_dim) {

        //----------------------------------------------------------------
        // configure col engines
        //----------------------------------------------------------------

        cE1_base[1] = B_ptr; B_ptr += C_stride;
        cE2_base[1] = B_ptr; B_ptr += C_stride;
        cE3_base[1] = B_ptr; B_ptr += C_stride;
        cE4_base[1] = B_ptr; B_ptr += C_stride;

        int round_elements = M * C_stride;
        cE1_base[2] = round_elements;
        cE2_base[2] = round_elements;
        cE3_base[2] = round_elements;
        cE4_base[2] = round_elements;

        cE1_base[3] = C_stride;
        cE2_base[3] = C_stride;
        cE3_base[3] = C_stride;
        cE4_base[3] = C_stride;

        int skip_addr = (P - C_stride + 1) * 4;
        cE1_base[4] = skip_addr;
        cE2_base[4] = skip_addr;
        cE3_base[4] = skip_addr;
        cE4_base[4] = skip_addr;

        int total_elements = round_elements * ceil_A_rows;
        cE1_base[5] = total_elements;
        cE2_base[5] = total_elements;
        cE3_base[5] = total_elements;
        cE4_base[5] = total_elements;

        cE1_base[0] = 1;
        cE2_base[0] = 1;
        cE3_base[0] = 1;
        cE4_base[0] = 1;

        //----------------------------------------------------------------
        // configure row engines
        //----------------------------------------------------------------

        int *A_ptr = matA_64;
        rE1_base[1] = A_ptr; A_ptr += engine_A_elements;
        rE2_base[1] = A_ptr; A_ptr += engine_A_elements;
        rE3_base[1] = A_ptr; A_ptr += engine_A_elements;
        rE4_base[1] = A_ptr;

        rE1_base[2] = engine_A_elements;
        rE2_base[2] = engine_A_elements;
        rE3_base[2] = engine_A_elements;
        rE4_base[2] = engine_A_elements;

        rE1_base[3] = M;
        rE2_base[3] = M;
        rE3_base[3] = M;
        rE4_base[3] = M;

        rE1_base[0] = 1;
        rE2_base[0] = 1;
        rE3_base[0] = 1;
        rE4_base[0] = 1;

        for (int x=0; x<tile_dim; ++x)
        for (int y=0; y<tile_dim; ++y) {
          bsg_remote_int_ptr base_ptr = bsg_global_ptr(1+x, 3+y, 0);
          base_ptr[1] = ceil_A_rows;
          base_ptr[2] = 0;
          base_ptr[4] = P * 4;
          base_ptr[5] = &(matC_64[y*ceil_A_rows*P + i + C_stride*x]);
          base_ptr[6] = C_stride;
          base_ptr[0] = 1;

        }

        // wait on row engines
        for (int y=0; y<4; ++y)
          wait_on_accelerator(0, 3+y);

        // wait on col engines
        for (int x=0; x<4; ++x)
          wait_on_accelerator(1+x, 2);

        // wait on accelerators
        for (int x=0; x<4; ++x)
        for (int y=0; y<4; ++y)
          wait_on_accelerator(1+x, 3+y);
      }
      bsg_print_time();

      for (int i=0; i<N_64*P_64; ++i)
      if (matC_64[i] != refC_64[i]) {
        bsg_printf("mat[%d][%d], %d != ref %d\n", i/P_64, i%P_64, matC_64[i], refC_64[i]);
        bsg_fail();
      }
    }
    bsg_printf(" [ passed ] \n");

    bsg_finish();
  }

  bsg_wait_while(1);
}

