////////////////////////////////////////////////////////////////////////////////////
//
//			VECTOR VECTOR AADD
//
///////////////////////////////////////////////////////////////////////////////////
//
//	AUTHOR: SHADY AGWA, shady.agwa@cornell.edu
//	DATE: 21 FEBREUARY 2019 
//	BASED ON PROF. BATTEN CODE
//	
//	The code add two vectors using 16 cores, Tile 0 intialize the two vectors,
//	then the 16 tiles exectute the addition in parallel (Vector_Size/16),
//	then Tile 0 verify the results and ends the execution!!
//
////////////////////////////////////////////////////////////////////////////////// 

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define TILE_DIM        4
#define NUM_X_TILES     TILE_DIM
#define NUM_Y_TILES     TILE_DIM



//------------------------------------------------------------------------
//// Global data
////------------------------------------------------------------------------

int size = 16;

int g_src0[256];
int g_src1[256];
int g_dest[256];

int g_go_flags[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int g_done_flags[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//------------------------------------------------------------------------
//// Vector Vector Add Function
////------------------------------------------------------------------------

void vvadd(int* dest, int* src0, int* src1, int size)
  {
    for ( int i = 0; i < size; i++)
      {
        *( dest + i ) = *( src0 + i ) + *( src1 + i );
      }	
   bsg_printf(".");

    // Set the done flag.
    g_done_flags[( ( bsg_x * TILE_DIM ) + bsg_y )] = 1;
  }

int main()
  {
    // Sets the bsg_x and bsg_y global variables.
    bsg_set_tile_x_y();

    int num_tiles = bsg_num_tiles;
    int tile_id   = bsg_x_y_to_id( bsg_x, bsg_y );
    
    // Determine where this tile should start in the data array.
    int start_id = tile_id*num_tiles;

    // Last tile will handle the remainder : Removed for Simplicity 
    // if ( tile_id == num_tiles-1 )
    // size = g_size % num_tiles; // may be size+=

    // Tile 0 will fill in the input data
    if ( tile_id == 0 ) {
        for ( int i = 0; i < g_size; i++ ) {
              g_src0[i] = i;
              g_src1[i] = i*10;
           
        }
        for ( int i = 1; i < num_tiles; i++ ) {
              g_go_flags[i] = 1;
        }

    }
    else {
	   int * flag = bsg_remote_ptr( 0, 0, &( g_go_flags[ tile_id ] ) );
	   while ( !( *( flag ) ) ) {
	           bsg_printf("."); 
	   }
    }
 
    // Execute vvadd for just this tile's partition.
    int * src0_pt = bsg_remote_ptr( 0, 0, &( g_src0[start_id] ) ) ;
    int * src1_pt = bsg_remote_ptr( 0, 0, &( g_src1[start_id] ) ) ;
    int * dest_pt = bsg_remote_ptr( 0, 0, &( g_dest[start_id] ) ) ;
    vvadd((dest_pt), (src0_pt), (src1_pt),size); 
     

    // Tile 0 will wait until all tiles are done.
    if( tile_id == 0 ) {
       for ( int i = 0; i < NUM_X_TILES; i++ )
       for ( int j = 0; j < NUM_Y_TILES; j++ ) {
         int * done  = bsg_remote_ptr( i, j, &( g_done_flags[(i*4)+j] ) ); 
         while ( !( *( done ) ) ) {
	         bsg_printf(".");
	 }
       }

    // Tile 0 will verify the results

      int passed = 1;
      for ( int i = 0; i < g_size; i++ ) {
        if ( g_dest[i] != ( i + i*10 ) ) {
             bsg_printf("\n\n *** FAILED *** g_dest[%d] incorrect, ( %d != %d ) \n\n", i, g_dest[i], i+i*10 );
             passed = 0;
             break;
        }
      }

      if ( passed ) {
          bsg_printf("\n\n *** _________# PASSED #_________ *** \n\n");
      }


        bsg_finish(); 
    }

  bsg_wait_while(1);
  
}
