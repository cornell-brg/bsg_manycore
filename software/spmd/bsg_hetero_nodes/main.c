
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"

// #define   BSG_TILE_GROUP_X_DIM  bsg_tiles_X
// #define   BSG_TILE_GROUP_Y_DIM  bsg_tiles_Y
// #include "bsg_tile_group_barrier.h"
// INIT_TILE_GROUP_BARRIER( row_barrier, col_barrier, 0, 0, bsg_tiles_X, bsg_tiles_Y);
/************************************************************************
 Declear an array in DRAM. 
*************************************************************************/
#define SUB_EPA_DIM 2
#define TOTAL_NUM   (SUB_EPA_DIM * bsg_tiles_X * bsg_tiles_Y)
// each tile will allocate SUB_EPA_DIM words, which is initilized to 
// (__bsg_y * bsg_tiles_X + __bsg_x) * SUB_EPA_DIM  + i;
int src_data[ SUB_EPA_DIM ] ;
void init_src_data( void  ) ;

int dst_data[ TOTAL_NUM ];

int done; 

#define  GS_X_CORD  0 
#define  GS_Y_CORD  (bsg_global_Y-1)
//--------------------------------------------------------------
// 1.  CSR definitions
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

typedef union {
        struct { 
                unsigned char epa_order       : 2;  
                unsigned char x_order         : 2;       
                unsigned char y_order         : 2;       
                unsigned int  padding         : (32 - 6);
        };
        unsigned  int val;
}dma_cmd_order_s;

//--------------------------------------------------------------
// 2. Funcion to gather data 
void bsg_set_param( 
                 Norm_NPA_s *   p_src 
                ,Norm_NPA_s *   p_dim 
                ,Norm_NPA_s *   p_incr 
                ,Norm_NPA_s *   p_signal
                ,unsigned int * p_local_dst
               );

void bsg_gather( dma_cmd_order_s * p_order
                ,Norm_NPA_s      * p_signal
                ,unsigned int    * p_local_dst
               );
//--------------------------------------------------------------
// 3.  Main function

bsg_remote_int_ptr GS_CSR_base_p; 
Norm_NPA_s  src_addr_s, src_dim_s, src_incr_s, sig_addr_s;
dma_cmd_order_s  dma_order_s;

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
  
  GS_CSR_base_p = bsg_global_ptr( GS_X_CORD, GS_Y_CORD, 0);


  // signal_addr_s.x_cord  = __bsg_x + __bsg_grp_org_x ;
  // signal_addr_s.y_cord  = __bsg_y + __bsg_grp_org_y ;
  // signal_addr_s.addr    = (short)( &done );

  init_src_data(); 
  //wait all the tiles has initilized the data
  //bsg_tile_group_barrier( &row_barrier, &col_barrier);

  if ((__bsg_x == bsg_tiles_X-1) && (__bsg_y == bsg_tiles_Y-1)) {
     /************************************************************************
       Concatenated Fetch
     *************************************************************************/
      //Configure the CSR 
      src_addr_s =(Norm_NPA_s)  {  .epa_addr    = (unsigned int)&src_data  / sizeof(int)
                                  ,.x_cord      = __bsg_grp_org_x 
                                  ,.y_cord      = __bsg_grp_org_y 
                                 };

      src_dim_s  =(Norm_NPA_s)  {  .epa_dim     =  SUB_EPA_DIM 
                                  ,.x_dim       =  bsg_tiles_X                
                                  ,.y_dim       =  bsg_tiles_Y
                                 };

      src_incr_s =(Norm_NPA_s)  {  .epa_incr    =  1
                                  ,.x_incr      =  1
                                  ,.y_incr      =  1
                                };

      sig_addr_s =(Norm_NPA_s)  {  .epa_addr    = (unsigned int)& done
                                  ,.x_cord      = __bsg_x + __bsg_grp_org_x 
                                  ,.y_cord      = __bsg_y + __bsg_grp_org_y 
                                 };

      dma_order_s = (dma_cmd_order_s) {         .epa_order = 0
                                               ,.x_order   = 1
                                               ,.y_order   = 2 
                                       };
      bsg_printf("Concatenated array data (should be re-ordered with load_id):\n");
      bsg_set_param(&src_addr_s, &src_dim_s, &src_incr_s, &sig_addr_s, &done); 
      bsg_gather(&dma_order_s, &sig_addr_s, &done); 
     /************************************************************************
       Interleaved Fetch
     *************************************************************************/

     dma_order_s = (dma_cmd_order_s) {          .epa_order = 2
                                               ,.x_order   = 0
                                               ,.y_order   = 1
                                       };
     bsg_printf("Interleaved array data (should be re-ordered with load_id):\n");
     bsg_gather(&dma_order_s,  &sig_addr_s, &done); 

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
                src_data[i] = (__bsg_y * bsg_tiles_X + __bsg_x) * SUB_EPA_DIM + i;
        }
        bsg_printf("First 2 data in tile (y,x)=(%d, %d)= %d, %d\n", __bsg_y, __bsg_x, src_data[0], src_data[1]);
}

void bsg_set_param( 
                 Norm_NPA_s *   p_src 
                ,Norm_NPA_s *   p_dim 
                ,Norm_NPA_s *   p_incr 
                ,Norm_NPA_s *   p_signal
                ,unsigned int * p_local_dst
               ){

      volatile int *GS_dst_ptr; 
      int i; 
      
      // Set CSR 
      * (GS_CSR_base_p + CSR_SRC_ADDR_HI_IDX )      =  p_src -> HI ;
      * (GS_CSR_base_p + CSR_SRC_ADDR_LO_IDX )      =  p_src -> LO ;
      
      * (GS_CSR_base_p + CSR_SRC_DIM_HI_IDX )       =  p_dim -> HI ;
      * (GS_CSR_base_p + CSR_SRC_DIM_LO_IDX )       =  p_dim -> LO ;
      
      * (GS_CSR_base_p + CSR_SRC_INCR_HI_IDX )      =  p_incr-> HI ;
      * (GS_CSR_base_p + CSR_SRC_INCR_LO_IDX )      =  p_incr-> LO ;
      
      * (GS_CSR_base_p + CSR_DST_ADDR_IDX   )       =  (int) p_local_dst;
      
      * (GS_CSR_base_p + CSR_SIG_ADDR_HI_IDX   )    =  p_signal-> HI;
      * (GS_CSR_base_p + CSR_SIG_ADDR_LO_IDX   )    =  p_signal-> LO;
}

void bsg_gather( dma_cmd_order_s * p_order
                ,Norm_NPA_s      * p_signal
                ,unsigned int    * p_local_dst
               ){

      volatile int *GS_dst_ptr; 
      int i; 
      
      // Clear the signal
      * (int *)( p_signal -> epa_addr )             =  0;
      // Fire the command
      * (GS_CSR_base_p + CSR_CMD_IDX )              =  p_order->val ;
      //wait the done signal.
      bsg_wait_local_int( (int *)( p_signal -> epa_addr ), 1);
      //print the result.
      GS_dst_ptr    = (volatile int *) bsg_global_ptr( GS_X_CORD, GS_Y_CORD, p_local_dst);

      for( i=0; i< TOTAL_NUM; i++){
                 bsg_printf("\t%d,", *(GS_dst_ptr+ i) );
      }
        
      bsg_printf("\n");
}

