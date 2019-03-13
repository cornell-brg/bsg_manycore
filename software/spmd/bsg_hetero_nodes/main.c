
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"

/************************************************************************
 Declear an array in DRAM. 
*************************************************************************/
#define SUB_EPA_DIM 2

// each tile will allocate SUB_EPA_DIM words, which is initilized to 
// (__bsg_x * bsg_tiles_Y + __bsg_y) * SUB_EPA_DIM  + i;
int src_data[ SUB_EPA_DIM ] ;
void init_src_data( void  ) ;

int dst_data[ SUB_EPA_DIM * bsg_tiles_X * bsg_tiles_Y ];

int done =0;

#define  GS_X_CORD  0 
#define  GS_Y_CORD  (bsg_global_Y-1)
//--------------------------------------------------------------
//  CSR definitions
 enum {
         CSR_CMD_IDX =0         //command, write to start the transcation
        ,CSR_SRC_ADDR_HI_IDX    //Source Address Configuration High, using Norm_NPA_s format
        ,CSR_SRC_ADDR_LO_IDX    //Source Address Configuration Low,  using Norm_NPA_s format
        ,CSR_SRC_DIM_HI_IDX     //Source Dimension Configuration High, using Norm_NPA_s format
        ,CSR_SRC_DIM_LO_IDX     //Source Dimension Configuration Low,  using Norm_NPA_s format
        ,CSR_SRC_INCR_HI_IDX    //Source Increasement Configuration High, using Norm_NPA_s format
        ,CSR_SRC_INCR_LO_IDX    //Source Increasement Configuration Low,  using Norm_NPA_s format

        ,CSR_DST_ADDR_IDX       //Local Desitination  addr
        ,CSR_SIG_ADDR_HI_IDX    //Signal Addr High, using Norm_NPA_s format
        ,CSR_SIG_ADDR_LO_IDX    //Signal Addr Low, using Norm_NPA_s format
        ,CSR_NUM_lp
} CSR_INDX;

typedef union {
        struct{ 
                union{                                  //LSB
                        unsigned int epa_incr ;
                        unsigned int epa_dim  ;
                        unsigned int epa_addr ;
                };
                union {
                        unsigned char x_incr    ;
                        unsigned char x_dim     ;
                        unsigned char x_cord    ;
                };
                union {
                        unsigned char y_incr    ;
                        unsigned char y_dim     ;
                        unsigned char y_cord    ;
                };
                unsigned char  chip_id  ;
                unsigned char  reserved ;               //MSB
        };
        struct{
               unsigned int LO;
               unsigned int HI;
        };
} Norm_NPA_s;

int main()
{
   int i, j;
  
    
   bsg_remote_int_ptr GS_CSR_base_p; 
   volatile int *GS_dst_ptr; 
  /************************************************************************
   This will setup the  X/Y coordination. Current pre-defined corrdinations 
   includes:
        __bsg_x         : The X cord inside the group 
        __bsg_y         : The Y cord inside the group
        __bsg_org_x     : The origin X cord of the group
        __bsg_org_y     : The origin Y cord of the group
  *************************************************************************/
  bsg_set_tile_x_y();
  
  // GS_CSR_base_p = bsg_global_ptr( GS_X_CORD, GS_Y_CORD, 0);

  // GS_dst_ptr    = (volatile int *) bsg_global_ptr( GS_X_CORD, GS_Y_CORD, dst_data);

  // signal_addr_s.x_cord  = __bsg_x + __bsg_grp_org_x ;
  // signal_addr_s.y_cord  = __bsg_y + __bsg_grp_org_y ;
  // signal_addr_s.addr    = (short)( &done );

  init_src_data(); 
 
  if ((__bsg_x == bsg_tiles_X-1) && (__bsg_y == bsg_tiles_Y-1)) {
     // //Configure the CSR, src[1][1]
     // * (GS_CSR_base_p + CSR_SRC_ADDR_IDX )      =  (int) (&src_data[1][1]);
     // // Y=31:16, X=15:0
     // * (GS_CSR_base_p + CSR_SRC_CORD_IDX )      =  ( ((bsg_global_Y)<<16) | 0x0 );
     // * (GS_CSR_base_p + CSR_1D_DIM_IDX   )      =  SUB_X_DIM * 4;
     // * (GS_CSR_base_p + CSR_2D_SKIP_IDX  )      =  Y_DIM * 4;
     // * (GS_CSR_base_p + CSR_2D_DIM_IDX   )      =  SUB_Y_DIM    ;
     // * (GS_CSR_base_p + CSR_DST_ADDR_IDX   )    =  (int) dst_data ;
     // * (GS_CSR_base_p + CSR_SIG_ADDR_IDX   )    =  signal_addr_s.val ;
     // * (GS_CSR_base_p + CSR_CMD_IDX )           =  1;

     // //wait the done signal.
     // bsg_wait_local_int( &done, 1);
     // bsg_printf("\nSource Matrix: \n");
     // for( i=0; i< X_DIM; i++){
     //    for(j=0; j< Y_DIM; j++)
     //            bsg_printf("\t%d,", src_data[i][j] );
     //    bsg_printf("\n");
     // }

     // bsg_printf("\nFetched Sub Matrix: \n");
     // for( i=0; i< SUB_X_DIM; i++){
     //    for(j=0; j< SUB_Y_DIM; j++)
     //            bsg_printf("\t%d,", *(GS_dst_ptr + i*SUB_Y_DIM + j) );
     //    bsg_printf("\n");
     // }
     /************************************************************************
       Terminates the Simulation
     *************************************************************************/
     bsg_finish();
  }

  bsg_wait_while(1);
}


void init_src_data( void ) {
        int i;
        for( i=0; i < SUB_EPA_DIM; i++){
                src_data[i] = (__bsg_x * bsg_tiles_Y + __bsg_y) * SUB_EPA_DIM + i;
        }
        bsg_printf("First 2 data in tile (%d, %d)= %d, %d\n", __bsg_x, __bsg_y, src_data[0], src_data[1]);
}
