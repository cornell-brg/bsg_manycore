//========================================================================
// Vector Vector Add [Runtime bthread]
//========================================================================
// The code add two vectors using ( X * Y ) cores and DRAM, Start Tile 0
// intializes two vectors,then the Tiles will exectute the addition, 
// in parallel. The last Tile will execute the remainder of the Vector.
// The Tile 0 verifies the results and ends the execution.
//
// Author: Shady Agwa, shady.agwa@cornell.edu
// Date: 5 March 2019.

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

//------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------

// Define Vectors in DRAM.
int g_src0[500] __attribute__ ( ( section(".dram") ) );
int g_src1[500] __attribute__ ( ( section(".dram") ) );
int g_dest[500] __attribute__ ( ( section(".dram") ) );
// Tiles Flags in SP, default value 0.
int g_thread_flags = 0;
// Vector Size Constant.
const int g_size = 500;
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

// Function pointer to be filled by Tile 0 for each Tile.
typedef void (*spawn_func_ptr)(void*);
spawn_func_ptr g_thread_spawn_func_ptrs;

//------------------------------------------------------------------------
// Start Execution & Intialize Inputs function
//------------------------------------------------------------------------

void thread_init()
{
  // Sets the bsg_x and bsg_y global variables.
  bsg_set_tile_x_y();
  int tile_id = bsg_x_y_to_id( bsg_x, bsg_y );
  // all Tiles except Tile 0 will be stuck here.
  if ( ( tile_id > 0 ) && ( tile_id < bsg_num_tiles ) ) {
    // Tiles will be in the worker loop.
    while (1) {
      // Wait until Tile 0 spawn function.
      while( g_thread_flags == 0 ) {
        __asm__ __volatile__ ( "nop;"
                               "nop;"
                               "nop;"
                               "nop;"
                               "nop;": : : "memory" );
      }
      // Execute the Tile's function pointer, using Tile's arguments.
      ( *g_thread_spawn_func_ptrs )( &( g_thread_spawn_func_args ) );
      // Set the flag so Tile 0 knows that this Tile is done.
      g_thread_flags = 0;
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
    int* flag = bsg_remote_ptr( tile_x, tile_y, &( g_thread_flags ) );
    if ( *( flag ) == 0 ) {
      // Set arguments pointer to a certain Tile X-Y.
      arg_t*  arg_ptr = bsg_remote_ptr( tile_x, tile_y, &( g_thread_spawn_func_args ) );
      *( arg_ptr ) = *( arg );
      // Wake up Tile 
      *( flag ) = 1;
    }
  }
}

//------------------------------------------------------------------------
// Join Function
//------------------------------------------------------------------------

void thread_join( int thread_id )
{
  if ( ( thread_id < bsg_num_tiles ) && ( thread_id > 0 ) ) {  
    int t_x = bsg_id_to_x( thread_id );
    int t_y = bsg_id_to_y( thread_id );
    int* flag = bsg_remote_ptr( t_x, t_y, &( g_thread_flags ) );
    // Wait until the Tile is no longer in use.
    while ( *( flag ) ) {
      __asm__ __volatile__ ( "nop;"
                             "nop;"
                             "nop;": : : "memory" );
    }
  }
}

//------------------------------------------------------------------------
// Verify Results function
//------------------------------------------------------------------------

void verify_results()
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
}

//------------------------------------------------------------------------
// main Function
//------------------------------------------------------------------------

int main()
{
  // Intialize Threads.
  thread_init();

  // Set the Functionality for all Tiles.
  g_thread_spawn_func_ptrs = &( vvadd_mt );

  // Tile 0 intializes input vectors.
  int t_id = bsg_x_y_to_id( bsg_x, bsg_y );
  if ( t_id == 0 ) {
    // Initialize Input Vectors -> to be moved!
    for ( int i = 0; i < g_size; i++ ) {
      g_src0[i] = i;
      g_src1[i] = i * 10;
    }
    // Tile 0 is working.
    g_thread_flags = 1;
  }                                                                  

  // Define start and end for each Tile's block size.
  int start;
  int end;
  // Have to use X & Y instead of Tile_id due to remote_store.
  for ( int k = 0; k < bsg_tiles_Y; k++ ) {
    for ( int j = 0; j < bsg_tiles_X; j++ ) {    
      int tile_id = ( 4 * k ) + j;
      start = ( tile_id * g_block_size );
      end = start + g_block_size;
      // Last Tile will consume the remainder, So end = g_size.
      if (tile_id == ( bsg_num_tiles - 1 ) ) {
        end = g_size; 
      }
      // Create arguments for functions working on other Tiles.
      if ( ( tile_id > 0 ) && ( tile_id < bsg_num_tiles ) ) {
        arg_t arg_i = { g_dest, g_src0, g_src1, start, end };
        // Spawn workload for Tile x = j, y = k. 
        thread_spawn( k , j, &( arg_i ) );
        // Set the function pointer of Tile x = j, y = k to the selected function.
        spawn_func_ptr* tile_fptr = bsg_remote_ptr( j, k, &( g_thread_spawn_func_ptrs ) );
        *( tile_fptr ) = g_thread_spawn_func_ptrs;
      }
    }
  } 
  // Create arguments for function working on Tile 0.
  arg_t arg_0 = { g_dest, g_src0, g_src1, 0, g_block_size };
  // Map work to Tile 0.
  (*g_thread_spawn_func_ptrs)( &( arg_0 ) ); 

  // Wait for other Tiles to finish.
  for ( int i = 1; i < bsg_num_tiles; i++ ) {
    thread_join(i);
  }

  // Tile 0 verifies Results.
  verify_results();

  // Tile 0 Ends Execution.
  bsg_finish();
  bsg_wait_while(1); 
  return 0;
}
