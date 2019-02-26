//========================================================================
// Vector Vector Add [Boxed]
//========================================================================
// The code add two vectors using ( X * Y ) cores and DRAM, Starting Tile
// intializes two vectors,then the Tiles will exectute the addition, 
// in parallel. The last Tile will execute the remainder of the Vector.
// The End Tile verifies the results and ends the execution.
//
// Author: Shady Agwa, shady.agwa@cornell.edu
// Date: 26 February 2019.

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

//------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------

// Define Vectors in DRAM.
int g_src0[500] __attribute__ ( ( section(".dram") ) );
int g_src1[500] __attribute__ ( ( section(".dram") ) );
int g_dest[500] __attribute__ ( ( section(".dram") ) );
// Define Go flag in DRAM.
int g_go_flag __attribute__ ( ( section(".dram") ) );
// Vector Size Constant.
const int g_size = 500;
// Run Size.
int run_size = ( g_size / ( bsg_tiles_X * bsg_tiles_Y ) );
// Done flag for all Cores.
int g_done_flag = 0;
// Select Tile 0 to start execution.
int s_tile_id = 0;
// Select Tile 15 to verify & end execution.
int e_tile_id = 15;

//------------------------------------------------------------------------
// Start Execution & Intialize Inputs function
//------------------------------------------------------------------------

void start_exe( int s_tile_id, int g_size )
{
  // Sets the bsg_x and bsg_y global variables.
  bsg_set_tile_x_y();
  int tile_id   = bsg_x_y_to_id( bsg_x, bsg_y );
  if ( tile_id == s_tile_id ) {
    for ( int i = 0; i < g_size; i++ ) {
      g_src0[i] = i;
      g_src1[i] = i * 10;
    }
    g_go_flag = 1;
  }
}

//------------------------------------------------------------------------
// Vector Vecctor Add function
//------------------------------------------------------------------------

void vvadd( int* dest, int* src0, int* src1, int size ) 
{
  for ( int i = 0; i < size; i++) {
    dest[i] = src0[i] + src1[i];
  }	
  // Trace Execution Phase.
  bsg_printf("e");
  // Set the done flag.
  g_done_flag = 1;
}

//------------------------------------------------------------------------
// Parallel Execution function
//------------------------------------------------------------------------

void parallel_exe( int* go, int size )
{
  int num_tiles = bsg_num_tiles;
  int tile_id   = bsg_x_y_to_id( bsg_x, bsg_y );
  // Determine where this tile should start in the data array.
  int start_id = tile_id * size;
  // Last tile will handle the remainder.
  if ( tile_id == ( num_tiles - 1 ) ) {
    size = size + ( g_size % num_tiles );
  }
  while ( *( go ) == 0 ) {
    // Waiting for go flag
    bsg_printf("w");
  }
  // Execute vvadd for this tile's partition.
  vvadd( &( g_dest[start_id] ), &( g_src0[start_id] ), &( g_src1[start_id] ), size );  
}

//------------------------------------------------------------------------
// Verify & End Execution function
//------------------------------------------------------------------------

void verify_end( int e_tile_id , int* g_done_flag )
{
  int tile_id   = bsg_x_y_to_id( bsg_x, bsg_y );
  if ( tile_id == e_tile_id ) {
    for ( int i = 0; i < bsg_tiles_X; i++ ) {
      for ( int j = 0; j < bsg_tiles_Y; j++ ) {
        int* done  = bsg_remote_ptr( i, j, &( g_done_flag ) );
        while ( !( *( done ) ) ) {
          // Waiting for all done flags.
          bsg_printf(".");
        }
      }
    }
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
}

//------------------------------------------------------------------------
// main Function
//------------------------------------------------------------------------

int main()
{
  // Start Execution from Tile s_tile_id to intitialize Vectors[g_size].
  start_exe( s_tile_id, g_size );
  // Parallel Execution starts executing when g_go_flag = 1 for run_size times.
  parallel_exe( &( g_go_flag ), run_size );
  // Verify & End execution by Tile e_tile_id when receiving all done flags.
  verify_end( e_tile_id, &( g_done_flag ) );

  bsg_wait_while(1); 
}
