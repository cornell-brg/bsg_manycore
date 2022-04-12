#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"
#include "ligra.h"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

struct BFS_F {
  uintE* Parents;
  BFS_F( uintE* _Parents ) : Parents( _Parents ) {}

  inline bool update( uintE s, uintE d )
  {
    if ( Parents[d] == UINT_E_MAX ) {
      Parents[d] = s;
      return 1;
    }
    else
      return 0;
  }

  inline bool updateAtomic( uintE s, uintE d )
  {
    return update( s, d );
  }

  // cond function checks if vertex has been visited yet
  inline bool cond( uintE d ) { return ( Parents[d] == UINT_E_MAX ); }
};

extern "C" __attribute__ ((noinline))
int kernel_appl_bfs(int* results, symmetricVertex* V, int n, int m, int* dram_buffer) {

  appl::runtime_init(dram_buffer, 16);

  // debug print
  if (__bsg_id == 0) {
    graph<symmetricVertex> G = graph<symmetricVertex>(V, n, m, nullptr);
    uint32_t idx = 0;
    for (uint32_t v = 0; v < G.n; v++) {
      for (uint32_t e = 0; e <  G.V[v].getOutDegree(); e++) {
        results[idx] = G.V[v].getOutNeighbor(e);
        idx++;
      }
    }

    size_t n     = G.n;
    size_t start = 0;
    uintE* Parents = newA( uintE, n );
    appl::parallel_for( size_t( 0 ), n,
                        [&]( size_t i ) { Parents[i] = UINT_E_MAX; } );
    Parents[start] = start;
    vertexSubset Frontier( n, start ); // creates initial frontier

    while ( !Frontier.isEmpty() ) {    // loop until frontier is empty
      vertexSubset output = edgeMap( G, Frontier, BFS_F( Parents ) );
      Frontier = output; // set new frontier
    }

    // print
    for (size_t i = 0; i < G.n; i++) {
      bsg_print_int(Parents[i]);
    }
  }

  appl::runtime_end();

  barrier.sync();
  return 0;
}
