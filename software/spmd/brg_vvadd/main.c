//========================================================================
// main.c
//========================================================================
// Author: Shady Agwa, shady.agwa@cornell.edu
// Date: 22 February 2019 
// Based on Prof. Batten Code
// The code add two vectors using 16 cores and DRAM, Tile 0 intializes the two vectors,
// then the 16 tiles will exectute the addition in parallel (Vector_Size/16), the last
// tile will execute the remainder of the Vector.
// Tile 0 verifies the results and ends the execution!!

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define TILE_DIM        4
#define NUM_X_TILES     TILE_DIM
#define NUM_Y_TILES     TILE_DIM

//------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------

// Define Vectors in DRAM.
int g_src0[256]  __attribute__ ((section (".dram")));
int g_src1[256]  __attribute__ ((section (".dram")));
int g_dest[256]  __attribute__ ((section (".dram")));
// Sizes of Vectors and loads of tiles.
const int g_size = 256;
int size = ( g_size / ( NUM_X_TILES * NUM_Y_TILES ) );
// Control Signals.
int* g_go_flag = 0;
int* g_done_flag = 0;

//------------------------------------------------------------------------
// Vector Vecctor Add function
//------------------------------------------------------------------------

void vvadd( int* dest, int* src0, int* src1, int size)
{
  for ( int i = 0; i < size; i++) {
    dest[i] = src0[i] + src1[i];
  }	
  // Trace Execution Phase.
  bsg_printf("E");
  // Set the done flag.
  int* done_xy = bsg_remote_ptr( bsg_x, bsg_y, &( g_done_flag ) );
  *( done_xy ) = 1;
}

//------------------------------------------------------------------------
// main Function
//------------------------------------------------------------------------

int main()
{
  // Sets the bsg_x and bsg_y global variables.
  bsg_set_tile_x_y();
  int num_tiles = bsg_num_tiles;
  int tile_id   = bsg_x_y_to_id( bsg_x, bsg_y );  
  // Determine where this tile should start in the data array.
  int start_id = tile_id * num_tiles;
  // Last tile will handle the remainder.
  if ( tile_id == ( num_tiles - 1 ) ) {
    size = size + ( g_size % num_tiles );
  }

  // Tile 0 will fill in the input data
  if ( tile_id == 0 ) {
    for ( int i = 0; i < g_size; i++ ) {
      g_src0[i] = i;
      g_src1[i] = i * 10;    
    }
    // write Tile 0 go flag for all tiles.
    int* go = bsg_remote_ptr( 0, 0, &( g_go_flag ) );
    *( go ) = 1;
  }
  else {
        int* go_t0 = bsg_remote_ptr( 0, 0, &( g_go_flag ) ) ;  
        while ( !( *( go_t0 ) ) ) {
          // Trace the Waiting Phase.
          bsg_printf("W"); 
    }
  }
 
  // Execute vvadd for just this tile's partition. 
  vvadd( &( g_dest[start_id] ), &( g_src0[start_id] ), &( g_src1[start_id] ), size );

  // Tile 0 will wait until all tiles are done.
  if ( tile_id == 0 ) {
    for ( int i = 0; i < NUM_X_TILES; i++ )
    for ( int j = 0; j < NUM_Y_TILES; j++ ) {
      int * done  = bsg_remote_ptr( i, j, &( g_done_flag ) ); 
      while ( !( *( done ) ) ) {
        bsg_printf("w");
      }
    }

    // Tile 0 will verify the results.
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
    bsg_finish(); 
  }

  bsg_wait_while(1); 
}
