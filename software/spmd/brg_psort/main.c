//========================================================================
// Parallel Sort 
//========================================================================
// Parallel sort using all tiles. Only sorts base 2 sized arrays
// Author: Xiaoyu Yan (xy97)
// Date: 7 June 2019.

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "data.h"
#include "bsg_barrier.h"
// #include "bsg_mutex.h"

//------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------

// Define Vectors in DRAM.
int g_src[128] __attribute__ ((section (".dram")));
int g_dest[128] __attribute__ ((section (".dram")));
// Size Variables & Constants.
const int g_size = 128; //size;
int size_per_tile = ( g_size / ( bsg_tiles_X * bsg_tiles_Y ) );
// Control Signals.
// int g_go_flag = 0;
// int g_done_flag = 0;

// Barrier signnals
#define BARRIER_X_START 0
#define BARRIER_Y_START 0

#define BARRIER_X_END (bsg_tiles_X - 1)
#define BARRIER_Y_END (bsg_tiles_Y - 1)
#define BARRIER_TILES ( (BARRIER_X_END +1) * ( BARRIER_Y_END+1) )
bsg_barrier  tile0_barrier = BSG_BARRIER_INIT(BARRIER_X_START, BARRIER_X_END,
BARRIER_Y_START, BARRIER_Y_END);

// Mutex Signals
bsg_mutex      tile0_mutex = 0;

//------------------------------------------------------------------------
// sort function
//------------------------------------------------------------------------

void swap ( int* arr, int i, int j) 
{
  int temp  = arr[i];
  arr[i]    = arr[j];
  arr[j]    = temp;
}

int partition( int* arr, int low, int high)
{
  int pivot = arr[high]; // Pivot is the right most element the sub array
  
  int i = low - 1; 
  for ( int j = low; j < high; j++) {
    if ( arr[j] <= pivot )
      if (i != j) {
        i++;
        swap(arr, i, j);
      }
  }
  swap(arr, i + 1, high); // swap first larger with pivot
  return (i + 1);
}

void quicksort_handler( int* arr, int low, int high)
{
  int p; // partition
  
  if ( low < high ) {
    p = partition(arr, low, high);
    
    quicksort_handler(arr, low, (p - 1));  // Sort Before p
    quicksort_handler(arr, (p + 1), high); // Sort After p
  }
}

void quicksort( int* dest, int* src, int size )
{
  quicksort_handler( dest, 0, size-1 );
  // Trace Execution Phase.
  bsg_printf("q");
  // print dest
}

// simplified merged sort in place with no copy on right side
void merge_sort( int *arr, int l, int m, int h )
{  
  int size1 = m-l, size2=h;
  int i, j;
  int left[size1];
  
  for(i = 0; i<size1; i++)
    left[i] = arr[i+l];

  i = 0;
  j = m;
  int k = l;
  
  //merge the left and right arrs in ascending order
  while (i<size1 && j <size2){
    if (left[i] < arr[j])
      arr[k++] = left[i++];
    else 
      arr[k++] = arr[j++];
  }
  
  //put the rest together
  while (i<size1)
    arr[k++] = left[i++];
  bsg_printf("m"); 

}

int verify_results( int dest[], int ref[], int size )
{
  int i;
  for ( i = 0; i < size; i++ ) {
    if ( !( dest[i] == ref[i] ) ) {
      bsg_printf("\n\n *** FAILED *** g_dest[%d] incorrect, ( %d != %d ) \n\n",
       i, dest[i], ref[i] );
      
      return 0;
    }
  }
  return 1;
}

//------------------------------------------------------------------------
// main Function
//------------------------------------------------------------------------
// Code loaded to every single tile
int main()
{
  // Sets the bsg_x and bsg_y global variables.
  bsg_set_tile_x_y();
  int num_tiles = bsg_num_tiles;
  int tile_id   = bsg_x_y_to_id( bsg_x, bsg_y );  
  // Determine where this tile should start in the data array.
  int start_id = tile_id * size_per_tile;
  
  bsg_mutex_ptr  p_mutex = ( bsg_mutex_ptr  ) 
  bsg_remote_ptr( 0, 0, (int *) (& tile0_mutex) );
  // Last tile will handle the remainder.
  if ( tile_id == ( num_tiles - 1 ) ) {
    size_per_tile = size_per_tile + ( g_size % num_tiles );
  }
  if ( tile_id == 0){
    for ( int i = 0; i < g_size; i++ ) {
      g_src[i] = src[i];
      g_dest[i] = g_src[i];
      // bsg_printf("%d ",src[i]);
    }
  }
  // bsg_printf("\n-----------id: %d arrived-----------\n",tile_id);
  bsg_barrier_wait( &tile0_barrier,BARRIER_X_START,BARRIER_Y_START);  
  // Execute sort for just this tile's partition. 
  quicksort( &(g_dest[start_id]), &(g_src[start_id]), size_per_tile);
  // bsg_barrier_wait( &tile0_barrier,BARRIER_X_START,BARRIER_Y_START);  
  // if (tile_id==0) {
  //   for (int j=0; j<g_size; j++){
  //     bsg_printf("%d ",g_dest[j]);
  //   }
  // }
  bsg_printf("\n-----------id: %d arrived-----------\n",tile_id);
  bsg_barrier_wait( &tile0_barrier,BARRIER_X_START,BARRIER_Y_START);  
  int n = 2;
  // Repeatedly mergesort until array is sorted
  while (BARRIER_TILES/n>0){
    if (tile_id < num_tiles/n) {
      // Use half the tiles each iteration but each tile sort twice
      // the amount
      int m,l,h;
      l = start_id * n;
      m = start_id * n + size_per_tile * n / 2;
      h = start_id * n + size_per_tile * n; 
      merge_sort(g_dest, l,m,h);
    }
    n = n * 2; 
    // bsg_barrier_wait( &tile0_barrier,0,0);
    // if (tile_id==0) {
    //   for (int j=0; j<g_size; j++){
    //     bsg_printf("%d ",g_dest[j]);
    //   }
    // 
    bsg_barrier_wait( &tile0_barrier,BARRIER_X_START,BARRIER_Y_START);  
    
  }

  // Set the done flag.
  
  // Tile 0 will wait until all tiles are done.
  // waits in order
  if ( tile_id == 0 ) {
    if ( verify_results(g_dest,ref,g_size) ) {
      bsg_printf("\n\n #### _________# PASSED #_________ #### \n\n");
    }
    bsg_finish(); 
  }
  
  bsg_wait_while(1); 
}
