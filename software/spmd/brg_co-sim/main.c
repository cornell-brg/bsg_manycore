
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"
#include "bsg_barrier.h"

// Barrier signnals
#define BARRIER_X_START 0
#define BARRIER_Y_START 0

#define BARRIER_X_END (bsg_tiles_X - 1)
#define BARRIER_Y_END (bsg_tiles_Y - 1)
#define BARRIER_TILES ( (BARRIER_X_END +1) * ( BARRIER_Y_END+1) )
bsg_barrier  tile0_barrier = BSG_BARRIER_INIT(BARRIER_X_START, BARRIER_X_END,
BARRIER_Y_START, BARRIER_Y_END);

#define CTRL_ADDR 0x00ffff0

#define COSIM_STOP 0b1000
#define COSIM_VAL 0b100
#define COSIM_GO  0b010
#define COSIM_DONE 0b001

/************************************************************************
 Declear an array in DRAM. 
*************************************************************************/
// int data[4] __attribute__ ((section (".dram")));//= { -1, 1, 0xF, 0x80000000};
bsg_mutex      tile0_mutex = 0;
int ctrl, cosim_val, cosim_go, cosim_done, 
cosim_stop; //ctrl bits
// {stop,val,go,done}

void cosim_init(  )
{
  bsg_dram_load((bsg_remote_int_ptr)CTRL_ADDR, ctrl);
  bsg_printf("init ctrl: %#x\n", ctrl);
  if (ctrl == 0)
    bsg_dram_store(CTRL_ADDR, COSIM_VAL);
}

void cosim_load_ctrl()
{ 
  bsg_dram_load((bsg_remote_int_ptr)CTRL_ADDR, ctrl);
  cosim_done = ctrl & COSIM_DONE;
  cosim_go = ( ctrl & COSIM_GO ) >> 1;
  cosim_val = ( ctrl & COSIM_VAL ) >> 2;
  cosim_stop = ( ctrl & COSIM_STOP ) >> 3;
  bsg_printf("\n-stop=%d;val=%d;go=%d;done=%d;\n",
  cosim_stop,
  cosim_val,
  cosim_go,
  cosim_done);
}

void cosim_write_ctrl()
{ //write relevant bits without touching the otherbits.
  int ctrl;
  if (cosim_done) ctrl |= cosim_done;
  else ctrl &= 0xfffffffe;

  if (cosim_go) ctrl |= (cosim_go << 1);
  else ctrl &= 0xfffffffd;

  if (cosim_val) ctrl |= (cosim_val << 2);
  else ctrl &= 0xfffffffb;

  if (cosim_stop) ctrl |= (cosim_stop << 3);
  else ctrl &= 0xfffffff7;
  bsg_printf("%s: writing_ctrl:%#x\n", __FILE__, ctrl);
  bsg_dram_store(CTRL_ADDR, ctrl);
}

void vvadd(int* src0, int* src1, int* dest, int size)
{
  for (int i = 0; i < size; i++){
    dest[i] = src0[i] + src1[i];
  }
}

int main()
{
  bsg_set_tile_x_y();
  bsg_printf("STARTING-");

  bsg_remote_int_ptr start_addr = (bsg_remote_int_ptr)0x0000fff0;
  bsg_mutex_ptr p_mutex = ( bsg_mutex_ptr  ) 
  bsg_remote_ptr( 0, 0, (int *) (& tile0_mutex) ); 
  int tile_id   = bsg_x_y_to_id( bsg_x, bsg_y ); 
  bsg_printf("\nManycore>> Hello from core %d, %d, id=%d, in group origin=(%d,%d).\n", \
                    __bsg_x, __bsg_y, tile_id, __bsg_grp_org_x, __bsg_grp_org_y);
  /************************************************************************
   This will setup the  X/Y coordination. Current pre-defined corrdinations 
   includes:
        __bsg_x         : The X cord inside the group 
        __bsg_y         : The Y cord inside the group
        __bsg_org_x     : The origin X cord of the group
        __bsg_org_y     : The origin Y cord of the group
  *************************************************************************/
  
  cosim_init();
  cosim_stop = 0;
  cosim_load_ctrl();
    
    while(!cosim_go){
      cosim_load_ctrl();
    }
    

  while (!cosim_stop){
    
    //SIM CODE

    bsg_mutex_lock(p_mutex);
    bsg_printf("\nValues in DRAM:\n");
    int read_value;
    // bsg_remote_int_ptr pData = bsg_dram_ptr( 0x000ffff0 ); 
    // int * arr1 = (int *) start_addr;
    // bsg_printf("\narry[2]:%d\n",pData[2]);

    for( bsg_remote_int_ptr j = start_addr; j < start_addr+16 ; j=j+1){
      bsg_dram_load( j, read_value);
      // bsg_remote_int_ptr pData = bsg_dram_ptr( j ); 
      bsg_printf("DRAM[%#08x]=%#08x\n",j,read_value);
      // bsg_printf("DRAM[%d]=%#08x\n",j,*pData);
    }
    bsg_printf("void cosim_write_ctrl stored in %#x\n",&cosim_write_ctrl);
    bsg_printf("void cosim_load_ctrl stored in %#x",&cosim_load_ctrl);
    // bsg_dram_load( CTRL_ADDR, read_value);
    // bsg_printf("ctrl value final [%#x]: %#x", CTRL_ADDR, read_value);

    bsg_printf("\n\n");
    bsg_mutex_unlock(p_mutex);

    bsg_barrier_wait( &tile0_barrier,BARRIER_X_START,BARRIER_Y_START);  
    // END SIM CODE
    
    cosim_load_ctrl();
    cosim_done = 1;
    cosim_go = 0;
    cosim_write_ctrl();
    cosim_load_ctrl();
  }
  
  bsg_finish();


  /************************************************************************
    Terminates the Simulation
  *************************************************************************/


  bsg_wait_while(1);
}

