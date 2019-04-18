
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

//dram_ch_addr_width_p set to DRAM_CH_ADDR_BITS-2
#define DRAM_CH_ADDR_BITS 18

// #include "dataset_tiny.dat"
// Define Vectors in DRAM.
int N_tiny = 3, M_tiny = 5, P_tiny = 4;
int matA_tiny[500] __attribute__ ( ( section(".dram") ) ) = {
  1,2,3,4,5,
  6,7,8,9,10,
  11,12,13,14,15
};
int matB_tiny[500] __attribute__ ( ( section(".dram") ) ) = {
  10,20,30,40,
  50,60,70,80,
  90,100,110,120,
  130,140,150,160,
  170,180,190,200
};
int refC_tiny[500] __attribute__ ( ( section(".dram") ) ) = {
  1750, 1900, 2050, 2200,
  4000, 4400, 4800, 5200,
  6250, 6900, 7550, 8200
};

inline void configure_row_engine(int x, int y, int *A_base_addr, int num_rows, int M) {
  bsg_remote_int_ptr base_ptr = bsg_global_ptr(x, y, 0);

  base_ptr[1] = A_base_addr;
  base_ptr[2] = num_rows*M;
  base_ptr[3] = M;
  base_ptr[0] = 1;
}

inline void configure_col_engine(int x, int y, int *B_base_addr, int N, int M, int P, int C_stride) {
  bsg_remote_int_ptr base_ptr = bsg_global_ptr(x, y, 0);

  base_ptr[1] = B_base_addr;
  base_ptr[2] = M * C_stride;
  base_ptr[3] = C_stride;
  base_ptr[4] = (P - C_stride + 1) * 4;
  base_ptr[5] = N * M * C_stride;
  base_ptr[0] = 1;
}

inline void configure_accelerator(int x, int y, int *C_base_addr, int n, int P, int actual_C_stride, int skip) {
  bsg_remote_int_ptr base_ptr = bsg_global_ptr(x, y, 0);

  base_ptr[1] = n;
  base_ptr[2] = skip;
  base_ptr[4] = P * 4;
  base_ptr[5] = C_base_addr;
  base_ptr[6] = actual_C_stride;
  base_ptr[0] = 1;
}
int wait_on_accelerator(int x, int y) {
  return bsg_global_ptr(x, y, 0)[0];
}

int matC_tiny[500] __attribute__ ( ( section(".dram") ) );

int main()
{
  bsg_set_tile_x_y();

  // Only core x,y=0,0 prepares data, since we set tile_group_Y = 1
  if ((__bsg_x == 0) && (__bsg_y == 0)) {

    if (bsg_global_X == 2 && bsg_global_Y == 3) { // 1x1 systolic
      int C_stride = 4;
      configure_row_engine(0, 2, matA_tiny, N_tiny, M_tiny);
      configure_col_engine(1, 1, matB_tiny, N_tiny, M_tiny, P_tiny, 4);
      configure_accelerator(1, 2, matC_tiny, N_tiny, P_tiny, C_stride, 0);

      wait_on_accelerator(1, 2);
      for (int i=0; i<N_tiny*P_tiny; ++i)
        bsg_printf("%d\n", matC_tiny[i]);
    }

    bsg_finish();
  }

  bsg_wait_while(1);
}

