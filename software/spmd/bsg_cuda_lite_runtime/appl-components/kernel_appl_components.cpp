#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"
#include "ligra.h"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

struct CC_F {
  uintE *IDs, *prevIDs;
  CC_F( uintE* _IDs, uintE* _prevIDs ) : IDs( _IDs ), prevIDs( _prevIDs )
  {
  }
  inline bool update( uintE s, uintE d ) const
  { // Update function writes min ID
    uintE origID = IDs[d];
    if ( IDs[s] < origID ) {
      IDs[d] = min( origID, IDs[s] );
      if ( origID == prevIDs[d] )
        return 1;
    }
    return 0;
  }
  inline bool updateAtomic( uintE s, uintE d ) const
  { // atomic Update
    uintE origID = IDs[d];
    return ( writeMinu( &IDs[d], IDs[s] ) && origID == prevIDs[d] );
  }
  inline bool cond( uintE d ) const { return cond_true( d ); } // does nothing
};

// function used by vertex map to sync prevIDs with IDs
struct CC_Vertex_F {
  uintE *IDs, *prevIDs;
  CC_Vertex_F( uintE* _IDs, uintE* _prevIDs )
      : IDs( _IDs ), prevIDs( _prevIDs )
  {
  }
  inline bool operator()( uintE i ) const
  {
    prevIDs[i] = IDs[i];
    return 1;
  }
};

template <class vertex>
void Compute( graph<vertex>& GA, int* results)
{
  size_t n   = GA.n;
  uintE* IDs = newA( uintE, n );
  uintE* prevIDs = newA( uintE, n );
  bool* frontier = newA( bool, n );
  appl::parallel_for( size_t( 0 ), n, [&]( size_t i ) {
        IDs[i] = i;
        frontier[i] = 1;
  } );
  vertexSubset Frontier(
      n, n, frontier ); // initial frontier contains all vertices

  while ( !Frontier.isEmpty() ) { // iterate until IDS converge
    vertexMap( Frontier, CC_Vertex_F( IDs, prevIDs ) );
    vertexSubset output = edgeMap( GA, Frontier, CC_F( IDs, prevIDs ) );
    Frontier.del();
    Frontier = output;
  }
  Frontier.del();

  for (size_t i = 0; i < n; i++) {
    results[i] = IDs[i];
  }
}

extern "C" __attribute__ ((noinline))
int kernel_appl_components(int* results, symmetricVertex* V, int n, int m, int* dram_buffer) {

  appl::runtime_init(dram_buffer, 16);
  barrier.sync();

  if (__bsg_id == 0) {
    graph<symmetricVertex> G = graph<symmetricVertex>(V, n, m, nullptr);
    Compute(G, results);
  } else {
    appl::worker_thread_init();
  }

  appl::runtime_end();

  barrier.sync();
  return 0;
}
