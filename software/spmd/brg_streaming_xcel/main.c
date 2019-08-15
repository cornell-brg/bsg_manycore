
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"
#include "streaming_xcel_test.dat"

enum {
  CSR_GO = 0, // go/fetch result
  CSR_DIM, // feeding A operand
  CSR_IN_ADDR,
  CSR_OUT_ADDR,
  CSR_block_M,
  CSR_block_N,
  CSR_block_P,
  CSR_M, // feeding B operand
  CSR_N,
  CSR_P,
} CSRs;

bsg_remote_int_ptr xcel_csr_base_ptr;

int main()
{   __asm__ __volatile__ ( "csrrwi s9,0x7c1, 0x1;"
                    "nop;"
                    "nop;"
                    "nop;"
                  );

  /************************************************************************
   This will setup the  X/Y coordination. Current pre-defined corrdinations
   includes:
        __bsg_x         : The X cord inside the group
        __bsg_y         : The Y cord inside the group
        __bsg_org_x     : The origin X cord of the group
        __bsg_org_y     : The origin Y cord of the group
  *************************************************************************/
  bsg_set_tile_x_y();

  if ((__bsg_x == 0) && (__bsg_y == 0)) {
    xcel_csr_base_ptr = bsg_global_ptr( 1, 1, 0);    
    xcel_csr_base_ptr[CSR_DIM    ] = 0;
    xcel_csr_base_ptr[CSR_IN_ADDR] = src;
    xcel_csr_base_ptr[CSR_OUT_ADDR] = dest;
    xcel_csr_base_ptr[CSR_block_M] = M;
    xcel_csr_base_ptr[CSR_block_N] = N;
    xcel_csr_base_ptr[CSR_block_P] = P;
    xcel_csr_base_ptr[CSR_M] = M;
    xcel_csr_base_ptr[CSR_N] = N;
    xcel_csr_base_ptr[CSR_P] = P;
    xcel_csr_base_ptr[CSR_GO] = 1;
    int done = xcel_csr_base_ptr[CSR_GO];
    __asm__ __volatile__ ( "csrrwi s9,0x7c1, 0x0;"
              );
    bsg_printf("d%d, in%x, out%x, bM%d, bN%d, bP%d, M%d, N%d, P%d, go%d\n",
    xcel_csr_base_ptr[CSR_DIM],xcel_csr_base_ptr[CSR_IN_ADDR],
    xcel_csr_base_ptr[CSR_OUT_ADDR],xcel_csr_base_ptr[CSR_block_M],
    xcel_csr_base_ptr[CSR_block_N],xcel_csr_base_ptr[CSR_block_P],
    xcel_csr_base_ptr[CSR_M],xcel_csr_base_ptr[CSR_N],xcel_csr_base_ptr[CSR_P],
    xcel_csr_base_ptr[CSR_GO]);
    int mismatch = 0;    
    for (int i = 0; i < N*P; i++){
      if (dest[i]!=ref[i]){
        bsg_printf("ERROR: dest[%d]=%d, expected %d\n",i,dest[i],ref[i]);
        mismatch = 1;
      }
    }
    if (mismatch) bsg_printf("###### TEST FAILED ######\n");
    else bsg_printf("###### TEST PASSED ######\n");
    // for (int i=0; i<4; ++i) {
    //   xcel_csr_base_ptr = bsg_global_ptr( i, 1, 0);

    //   xcel_csr_base_ptr[CSR_OPA] = data_gcd_A[i];
    //   xcel_csr_base_ptr[CSR_OPB] = data_gcd_B[i];
    //   xcel_csr_base_ptr[CSR_GO]  = 1;

    //   bsg_printf("xcel #%d returns gcd(%d,%d) = %d | ref = %d\n",
    //              i, data_gcd_A[i], data_gcd_B[i],
    //              xcel_csr_base_ptr[CSR_GO], data_gcd_ref[i]);

    /************************************************************************
      Terminates the Simulation
    *************************************************************************/

    bsg_finish();
  }

  bsg_wait_while(1);
}

