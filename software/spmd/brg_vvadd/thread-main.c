//========================================================================
// Vector Vector Add [Runtime thread-based]
//========================================================================
// The code add two vectors using ( X * Y ) cores and DRAM, Start Tile 0
// intializes two vectors,then the Tiles will exectute the addition, 
// in parallel. The last Tile will execute the remainder of the Vector.
// The End Tile 0 verifies the results and ends the execution.
//
// Author: Shady Agwa, shady.agwa@cornell.edu
// Date: 28 February 2019.

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

//------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------

// Define Vectors in DRAM.
int g_src0[256] __attribute__ ( ( section(".dram") ) );
int g_src1[256] __attribute__ ( ( section(".dram") ) );
int g_dest[256] __attribute__ ( ( section(".dram") ) );
// Tiles Flags in DRAM.
int g_thread_flags[bsg_num_tiles] __attribute__ ( ( section(".dram") ) );
// Vector Size Constant.
const int g_size = 256;
// Run Size.
int g_block_size = ( g_size / ( bsg_tiles_X * bsg_tiles_Y ) );

//------------------------------------------------------------------------
// Argument struct
//------------------------------------------------------------------------

typedef struct {
  int* dest;  // pointer to dest array
  int* src0;  // pointer to src0 array
  int* src1;  // pointer to src1 array
  int  begin; // first element this core should process
  int  end;   // last element + 1 this core should process
} arg_t;

typedef void (*spawn_func_ptr)(void*);
spawn_func_ptr g_thread_spawn_func_ptrs[bsg_num_tiles] __attribute__ ( ( section(".dram") ) ); // Defined in Tiles DRAM
void* g_thread_spawn_func_args[bsg_num_tiles] __attribute__ ( ( section(".dram") ) ); // Defined in SP

//------------------------------------------------------------------------
// Start Execution & Intialize Inputs function
//------------------------------------------------------------------------

void thread_init()
{
  // Sets the bsg_x and bsg_y global variables.
  bsg_set_tile_x_y();
  int tile_id   = bsg_x_y_to_id( bsg_x, bsg_y );
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
    // Tiles  will be  in the worker loop.
    while (1) {
      // Wait until Tile 0 spawn function.
      while( g_thread_flags[tile_id] == 0 ) {
        bsg_printf("");
      }
      // Execute the spawn function.
      // spawn_func_ptr f_ptr = bsg_remote_ptr( 0, 0, &( g_thread_spawn_func_ptrs[tile_id] ) );
      // void*  arg_ptr = bsg_remote_ptr( 0, 0, &( g_thread_spawn_func_args[tile_id] ) );
      //vvadd_mt( &g_thread_spawn_func_args[tile_id] );
      (*g_thread_spawn_func_ptrs[tile_id])( g_thread_spawn_func_args[tile_id] );

      // Unset the flag so Tile 0  knows that this Tile is done.
      g_thread_flags[tile_id] = 0;
    }
  }
}

//------------------------------------------------------------------------
// Vector Vecctor Add function
//------------------------------------------------------------------------

void vvadd_mt( void* arg_vptr )
{
  // Cast void* to arg_ptr
  arg_t* arg_ptr = (arg_t*) arg_vptr;
  // create local variables of the argument structure for this Tile
  int* dest  = arg_ptr->dest;
  int* src0  = arg_ptr->src0;
  int* src1  = arg_ptr->src1;
  int  begin = arg_ptr->begin;
  int end = arg_ptr->end;
  // Do the Addition.
  for ( int i = begin; i < end; i++ ) {
    dest[i] = src0[i] + src1[i];
  }
  // bsg_printf("%d %d \n ", src0[begin], src1[begin]);
}

//------------------------------------------------------------------------
// Spawn Function
//------------------------------------------------------------------------

void thread_spawn( int thread_id, void (*start_routine)(void*), void* arg )
{
  if ( ( thread_id < bsg_num_tiles ) && ( thread_id > 0 ) ) {
    if ( g_thread_flags[thread_id] == 0 ) {
      // Set function and argument pointer
      g_thread_spawn_func_args[thread_id] = arg;
      g_thread_spawn_func_ptrs[thread_id] = start_routine;
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
      bsg_printf("");
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
  for ( int i = 1; i < bsg_num_tiles; i++ ) {
    start = ( i * g_block_size );
    if (i == ( bsg_num_tiles - 1 ) ) {
      end = g_size; 
    }
    else {
      end = start + g_block_size;
    };
    // Create argument for function working on Tile i.
    arg_t arg_i = { g_dest, g_src0, g_src1, start, end };
    // Spawn work onto Tile i. 
    thread_spawn( i, &( vvadd_mt ), &( arg_i ) );
  } 
  // Create argument for function working on Tile 0.
  arg_t arg_0 = { g_dest, g_src0, g_src1, 0, g_block_size };
  bsg_printf("\n %d_%d \n ", bsg_x, bsg_y);
  // Spawn work onto Tile i.
  vvadd_mt( &( arg_0 ) );
  // Wait for other Tiles to finish.
  for ( int i = 1; i < bsg_num_tiles; i++ ) {
    thread_join(i);
    bsg_printf("\n Tile %d joined \n", i);
  }
  // Tile 0 verifies Results & End execution.
  if ( bsg_x_y_to_id( bsg_x, bsg_y ) == 0 ) {
    verify_end();
  }
  bsg_wait_while(1); 
}
