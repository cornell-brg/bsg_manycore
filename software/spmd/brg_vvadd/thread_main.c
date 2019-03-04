//========================================================================
// Vector Vector Add [Runtime bthread-like]
//========================================================================
// The code add two vectors using ( X * Y ) cores and DRAM, Start Tile 0
// intializes two vectors,then the Tiles will exectute the addition, 
// in parallel. The last Tile will execute the remainder of the Vector.
// The Tile 0 verifies the results and ends the execution.
//
// Author: Shady Agwa, shady.agwa@cornell.edu
// Date: 28 February 2019.

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

//------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------

// Define Vectors in DRAM.
int g_src0[550] __attribute__ ( ( section(".dram") ) );
int g_src1[550] __attribute__ ( ( section(".dram") ) );
int g_dest[550] __attribute__ ( ( section(".dram") ) );
// Tiles Flags in DRAM.
int g_thread_flags[bsg_num_tiles] __attribute__ ( ( section(".dram") ) );
// Vector Size Constant.
const int g_size = 550;
// Run Size Per each Tile.
int g_block_size = ( g_size / ( bsg_tiles_X * bsg_tiles_Y ) );

//------------------------------------------------------------------------
// Argument struct
//------------------------------------------------------------------------

typedef struct {
  int* dest; // pointer to dest array
  int* src0; // pointer to src0 array
  int* src1; // pointer to src1 array
  int begin; // first element this core should process
  int end;   // last element + 1 this core should process
} arg_t;

// Function arguments to be sent to different Tiles.
arg_t* g_thread_spawn_func_args; 

//------------------------------------------------------------------------
// Start Execution & Intialize Inputs function
//------------------------------------------------------------------------

void thread_init()
{
  // Sets the bsg_x and bsg_y global variables.
  bsg_set_tile_x_y();
  int tile_id = bsg_x_y_to_id( bsg_x, bsg_y );
  // Tile 0 intialize input vectors & Tile flags
  if ( tile_id == 0 ) {
    for ( int i = 0; i < bsg_num_tiles; i++ ) {
      g_thread_flags[i] = 0;
    }
    for ( int i = 0; i < g_size; i++ ) {
      g_src0[i] = i;
      g_src1[i] = i * 10;
    }
    // Tile 0 is working.
    g_thread_flags[0] = 1;
  }
  else {
    // Tiles will be in the worker loop.
    while (1) {
      // Wait until Tile 0 spawn function.
      while( g_thread_flags[tile_id] == 0 ) {
        __asm__ __volatile__ ( "nop;"
                               "nop;"
                               "nop;": : : "memory" );
      }
      // Execute the vector addition function, use Tile's arguments.
      vvadd_mt( &( g_thread_spawn_func_args ) );
      // Set the flag so Tile 0 knows that this Tile is done.
      g_thread_flags[tile_id] = 0;
    }
  }
}

//------------------------------------------------------------------------
// Vector Vecctor Add function
//------------------------------------------------------------------------

void vvadd_mt( arg_t* arg_vptr )
{
  // create local variables of the argument structure for this Tile
  int* dest = arg_vptr->dest;
  int* src0 = arg_vptr->src0;
  int* src1 = arg_vptr->src1;
  int begin = arg_vptr->begin;
  int end = arg_vptr->end;
  // Do the Addition.
  for ( int i = begin; i < end; i++ ) {
    dest[i] = src0[i] + src1[i];
  }
}

//------------------------------------------------------------------------
// Spawn Function
//------------------------------------------------------------------------

void thread_spawn( int tile_y, int tile_x, arg_t* arg )
{
  int thread_id = bsg_x_y_to_id( tile_x, tile_y );
  if ( ( thread_id < bsg_num_tiles ) && ( thread_id > 0 ) ) {
    if ( g_thread_flags[thread_id] == 0 ) {
      // Set arguments pointer to a certain Tile X-Y.
      arg_t*  arg_ptr = bsg_remote_ptr( tile_x, tile_y, &( g_thread_spawn_func_args ) );
      *( arg_ptr ) = *( arg );
      // Wake up Tile 
      g_thread_flags[thread_id] = 1;
    }
  }
}

//------------------------------------------------------------------------
// Join Function
//------------------------------------------------------------------------

void thread_join( int thread_id )
{
  if ( ( thread_id < bsg_num_tiles ) && ( thread_id > 0 ) ) {  
    // Wait until the Tile is no longer in use.
    while ( g_thread_flags[thread_id] ) {
      __asm__ __volatile__ ( "nop;"
                             "nop;"
                             "nop;": : : "memory" );
    }
  }
}

//------------------------------------------------------------------------
// Verify & End Execution function
//------------------------------------------------------------------------

void verify_end()
{
  // Verify Results.
  int passed = 1;
  for ( int i = 0; i < g_size; i++ ) {
    if ( g_dest[i] != ( i + i*10 ) ) {
      bsg_printf("\n\n *** FAILED *** g_dest[%d] incorrect, ( %d != %d ) \n\n", i, g_dest[i], i+i*10 );
      passed = 0;
      break;
    }
  } 
  if ( passed ) {
    bsg_printf("\n\n #### _________# PASSED #_________ #### \n\n");
  }
  // End Execution.
  bsg_finish();
}

//------------------------------------------------------------------------
// main Function
//------------------------------------------------------------------------

int main()
{
  // Intialize Threads.
  thread_init();
  // Define start and end for each Tile.
  int start;
  int end;
  // Have to use X & Y instead of Tile_id due to remote_store.
  for ( int k = 0; k < bsg_tiles_Y; k++ ) {
    for ( int j = 0; j < bsg_tiles_X; j++ ) {    
      int tile_id = ( 4 * k ) + j;
      start = ( tile_id * g_block_size );
      // Last Tile will consume the remainder, So end = g_size.
      if (tile_id == ( bsg_num_tiles - 1 ) ) {
        end = g_size; 
      }
      else {
        end = start + g_block_size;
      }
      // Create arguments for functions working on other Tiles.
      if ( ( tile_id > 0 ) && ( tile_id < bsg_num_tiles ) ) {
        arg_t arg_i = { g_dest, g_src0, g_src1, start, end };
        // Spawn workload for Tile x = j, y = k. 
        thread_spawn( k , j, &( arg_i ) );
      }
    }
  } 
  // Create arguments for function working on Tile 0.
  arg_t arg_0 = { g_dest, g_src0, g_src1, 0, g_block_size };
  // Map work to Tile 0.
  vvadd_mt( &( arg_0 ) );
  // Wait for other Tiles to finish.
  for ( int i = 1; i < bsg_num_tiles; i++ ) {
    thread_join(i);
  }
  // Tile 0 verifies Results & End execution.
  verify_end();
  bsg_wait_while(1); 
  return 0;
}
