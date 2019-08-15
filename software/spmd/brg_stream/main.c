/**
 * Xiaoyu Yan (xy97) 07/25/19
 * Cosim implementation of sum for matrices
 */

#include "streaming_test.dat"
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#define BSG_TILE_GROUP_X_DIM bsg_tiles_X
#define BSG_TILE_GROUP_Y_DIM bsg_tiles_Y
#include "bsg_tile_group_barrier.h"
INIT_TILE_GROUP_BARRIER(r_barrier, c_barrier, 0, bsg_tiles_X-1, 0, bsg_tiles_Y-1);
//------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------


int  __attribute__ ((noinline)) kernel_sum_out(int *A, int *C,
 int M, int N, int P, int block_M, int block_P, int block_N, int dim) {
   __asm__ __volatile__ ( "csrrwi s9,0x7c1, 0x1;"
                         "nop;"
                         "nop;"
                         "nop;"
                       );
  int sum[P];
  int bsg_id = bsg_x_y_to_id(__bsg_x,__bsg_y);
  for (int n = bsg_id; n < N; n+=bsg_num_tiles ) {
    for (int p = 0; p < P; p++){
      sum[p] = A[p + n*P];
    }
    for (int m = 1; m < M; m++) {
      for (int p = 0; p < P; p++){
        sum[p] += A[p + n*P + m*P*N];
        // bsg_printf("A[%d]=%d\n",p + n*P + m*P*N,A[p + n*P + m*P*N]);
        // bsg_printf("sum[%d]=%d\n",p,sum[p]);
      }
    }
    for ( int p = 0; p < P; p++){
      C[p + n*P] = sum[p];
      // bsg_printf("C[%d]=%d\n",p + n*P, C[p + n*P]);
    }
    
  }
  
  // // Barrier to signal completion.
  bsg_tile_group_barrier(&r_barrier, &c_barrier);
  __asm__ __volatile__ ( "csrrwi s9,0x7c1, 0x0;"
                       );
  return 0;
}

//------------------------------------------------------------------------
// main Function
//------------------------------------------------------------------------

int main()
{
  bsg_set_tile_x_y();

  kernel_sum_out(src,dest,M,N,P,M,N,P,0);
  // Tile 0 will wait until all tiles are done.
  if ((__bsg_x == 0) && (__bsg_y == 0)) {
    // Tile 0 will verify the results.
    int passed = 1;
    for ( int i = 0; i < N*P; i++ ) {
      if ( ref[i] != dest[i] ) {
        bsg_printf("\n\n *** FAILED *** g_dest[%d] incorrect, ( %d != %d ) \n\n", i, dest[i], ref[i] );
        passed = 0;
      }
    }
    if ( passed ) bsg_printf("\n\n #### _________# PASSED #_________ #### \n\n");
    else bsg_printf("###### TEST FAILED ######\n");
    bsg_finish(); 
  }

  bsg_wait_while(1); 
}
