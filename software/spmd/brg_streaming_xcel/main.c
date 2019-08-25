
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"
#include "streaming_xcel_test.dat"

enum {
  CSR_GO = 0, // go/fetch result
  CSR_DIM = 1, 
  CSR_IN_ADDR = 2,
  CSR_OUT_ADDR = 3,
  CSR_STRIDE = 4,
  CSR_IN_ADDR_END = 5,
  CSR_BOUND_ADDR = 6,
  CSR_NUM_ELEMENTS = 7, 
  CSR_COUNT = 10
} CSRs;

bsg_remote_int_ptr xcel_csr_base_ptr;

int main()
{   __asm__ __volatile__ ( "csrrwi s9,0x7c1, 0x1;"
                    "nop;"
                    "nop;"
                    "nop;"
                  );
  bsg_set_tile_x_y();
  int block_N = N;
  int block_P = P;

  if ((__bsg_x == 0) && (__bsg_y == 0)) {
    xcel_csr_base_ptr = bsg_global_ptr( 0, 2, 0);
        
    xcel_csr_base_ptr[CSR_COUNT    ] = 1;
    xcel_csr_base_ptr[CSR_DIM    ] = 0;
    xcel_csr_base_ptr[CSR_IN_ADDR] = (int)src;
    xcel_csr_base_ptr[CSR_OUT_ADDR] = (int)dest;
    xcel_csr_base_ptr[CSR_STRIDE] = N*P*4; // Multiplied by 4 for 4 bytes
    xcel_csr_base_ptr[CSR_IN_ADDR_END] = ((int)src + N*P*(M-1)*4);
    xcel_csr_base_ptr[CSR_BOUND_ADDR] = ((int)src + P*block_N*4);
    xcel_csr_base_ptr[CSR_NUM_ELEMENTS] = block_N*block_P;
    xcel_csr_base_ptr[CSR_GO] = 1;
    int done = xcel_csr_base_ptr[CSR_GO];
    xcel_csr_base_ptr[CSR_COUNT    ] = 0;
    __asm__ __volatile__ ( "csrrwi s9,0x7c1, 0x0;"
              );
    bsg_printf("d:%d, in:%x, out:%x, str:%d, inE:%x, bA:%x, #E:%d, M%d, N%d, P%d\n",
    xcel_csr_base_ptr[CSR_DIM],xcel_csr_base_ptr[CSR_IN_ADDR],
    xcel_csr_base_ptr[CSR_OUT_ADDR],xcel_csr_base_ptr[CSR_STRIDE],
    xcel_csr_base_ptr[CSR_IN_ADDR_END],xcel_csr_base_ptr[CSR_BOUND_ADDR],
    xcel_csr_base_ptr[CSR_NUM_ELEMENTS], M, N, P);
    int mismatch = 0;    
    for (int y = 0; y < N; y ++) { 
      for (int x = 0; x < P; x ++) { 
        if (ref[y * P + x] != dest[y * P + x]) {
          mismatch = 1;
        }
      }
    }
    // bsg_printf("result = [\n");
    // for (int y = 0; y < N; y ++) { 
    //   for (int x = 0; x < P; x ++) { 
    //     if (ref[y * P + x] != dest[y * P + x]) {
    //       bsg_printf("% 5d ", dest[y * P + x]);
    //       mismatch = 1;
    //     }
    //     else {
    //       bsg_printf("% 5d ", dest[y * P + x]);
    //     }
    //   }
    //   bsg_printf("\n");
    // }
    // bsg_printf("\n");
    // bsg_printf("]\n");
    // if (mismatch) {
    //   bsg_printf("ref = [\n");
    //   for (int y = 0; y < N; y ++) { 
    //     for (int x = 0; x < P; x ++) { 
    //       bsg_printf("% 5d ", ref[y * P + x]);
    //     }
    //     bsg_printf("\n");
    //   }
    //   bsg_printf("]\n");
    // }
    if (mismatch) bsg_printf("###### TEST FAILED ######\n");
    else bsg_printf("###### TEST PASSED ######\n");

    /************************************************************************
      Terminates the Simulation
    *************************************************************************/

    bsg_finish();
  }

  bsg_wait_while(1);
}

