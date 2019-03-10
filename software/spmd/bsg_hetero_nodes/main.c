
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

/************************************************************************
 Declear an array in DRAM. 
*************************************************************************/
int data[4] __attribute__ ((section (".dram"))) = { -1, 1, 0xF, 0x80000000};

#define  GS_X_CORD  0 
#define  GS_Y_CORD  (bsg_global_Y-1)

int main()
{
   int i, tmp;
   
   bsg_remote_int_ptr GS_CSR_base_p; 
  /************************************************************************
   This will setup the  X/Y coordination. Current pre-defined corrdinations 
   includes:
        __bsg_x         : The X cord inside the group 
        __bsg_y         : The Y cord inside the group
        __bsg_org_x     : The origin X cord of the group
        __bsg_org_y     : The origin Y cord of the group
  *************************************************************************/
  bsg_set_tile_x_y();
  
  GS_CSR_base_p = bsg_global_ptr( GS_X_CORD, GS_Y_CORD, 0);

  if ((__bsg_x == bsg_tiles_X-1) && (__bsg_y == bsg_tiles_Y-1)) {

     * GS_CSR_base_p = 0x1;
     tmp             = * GS_CSR_base_p;
     bsg_printf("\nManycore>> CSR[0] = %x\n",  tmp);
  /************************************************************************
    Terminates the Simulation
  *************************************************************************/
    bsg_finish();
  }

  bsg_wait_while(1);
}

