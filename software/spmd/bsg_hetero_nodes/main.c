
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"

/************************************************************************
 Declear an array in DRAM. 
*************************************************************************/
#define X_DIM 4
#define Y_DIM 4
#define SUB_X_DIM 2
#define SUB_Y_DIM 2
int src_data[ X_DIM ][ Y_DIM]  __attribute__ ((section (".dram"))) = 
       {  { 1, 2,   3, 4 }
         ,{ 5, 6,   7, 8 }
         ,{ 9, 10, 11, 12}
         ,{ 13,14, 15, 16}
       };

int dst_data[ SUB_X_DIM ] [ SUB_Y_DIM ];

int done =0;

#define  GS_X_CORD  0 
#define  GS_Y_CORD  (bsg_global_Y-1)
//--------------------------------------------------------------
//  CSR definitions
 enum {
     CSR_CMD_IDX =0         //command, write to start the transcation
    ,CSR_STATUS_IDX         //the status, 0: idle, 1: running
    ,CSR_SRC_ADDR_IDX       //source addr
    ,CSR_SRC_CORD_IDX       //the source x/y cord, X==15:0, Y=31:16
    ,CSR_1D_DIM_IDX         //legnth in byte for the 1st dimension
    ,CSR_2D_SKIP_IDX        //2D skip in bytes.
    ,CSR_2D_DIM_IDX         //2D dimension
    ,CSR_DST_ADDR_IDX       //dest addr
    ,CSR_SIG_ADDR_IDX       //the signal addr, write non-zero to indicates finish
    ,CSR_NUM_lp
} CSR_INDX;

int main()
{
   int i, tmp;
   
   bsg_remote_int_ptr GS_CSR_base_p ; 
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
     //Configure the CSR, src[1][1]
     * (GS_CSR_base_p + CSR_SRC_ADDR_IDX )      =  (int) (&src_data[1][1]);
     // Y=31:16, X=15:0
     * (GS_CSR_base_p + CSR_SRC_CORD_IDX )      =  ( ((bsg_global_Y)<<16) | 0x0 );
     * (GS_CSR_base_p + CSR_1D_DIM_IDX   )      =  SUB_X_DIM * 4;
     * (GS_CSR_base_p + CSR_2D_SKIP_IDX  )      =  Y_DIM * 4;
     * (GS_CSR_base_p + CSR_2D_DIM_IDX   )      =  SUB_Y_DIM    ;
     * (GS_CSR_base_p + CSR_CMD_IDX )           =  1;
     /************************************************************************
       Terminates the Simulation
     *************************************************************************/
  //  bsg_finish();
  }

  bsg_wait_while(1);
}

