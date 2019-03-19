
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"

enum {
  CSR_GO = 0, // go/fetch result
  CSR_OPA, // feeding A operand
  CSR_OPB, // feeding B operand
  CSR_NUM
} CSRs;

bsg_remote_int_ptr xcel_csr_base_ptr;

// directed test
int data_gcd_A[]   = { 42, 461952, 7966496,   45296490 };
int data_gcd_B[]   = { 56, 116298, 314080416, 24826148 };
int data_gcd_ref[] = { 14, 18,     32,        526      };

int main()
{
   int i, j;

  /************************************************************************
   This will setup the  X/Y coordination. Current pre-defined corrdinations
   includes:
        __bsg_x         : The X cord inside the group
        __bsg_y         : The Y cord inside the group
        __bsg_org_x     : The origin X cord of the group
        __bsg_org_y     : The origin Y cord of the group
  *************************************************************************/
  bsg_set_tile_x_y();

  if ((__bsg_x == bsg_tiles_X-1) && (__bsg_y == bsg_tiles_Y-1)) {

    for (int i=0; i<4; ++i) {
      xcel_csr_base_ptr = bsg_global_ptr( i, 3, 0);

      xcel_csr_base_ptr[CSR_OPA] = data_gcd_A[i];
      xcel_csr_base_ptr[CSR_OPB] = data_gcd_B[i];
      xcel_csr_base_ptr[CSR_GO]  = 1;

      bsg_printf("xcel #%d returns gcd(%d,%d) = %d | ref = %d\n",
                 i, data_gcd_A[i], data_gcd_B[i],
                 xcel_csr_base_ptr[CSR_GO], data_gcd_ref[i]);

    /************************************************************************
      Terminates the Simulation
    *************************************************************************/
    }
    bsg_finish();
  }

  bsg_wait_while(1);
}

