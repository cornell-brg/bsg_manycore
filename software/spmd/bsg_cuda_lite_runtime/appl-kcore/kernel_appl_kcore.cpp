#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"
#include "ligra.h"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

struct Update_Deg {
  intE* Degrees;
  Update_Deg( intE* _Degrees ) : Degrees( _Degrees ) {}
  inline bool update( uintE s, uintE d ) const
  {
    Degrees[d]--;
    return 1;
  }
  inline bool updateAtomic( uintE s, uintE d ) const
  {
    writeAdd( &Degrees[d], intE(-1) );
    return 1;
  }
  inline bool cond( uintE d ) const { return Degrees[d] > 0; }
};

template <class vertex>
struct Deg_LessThan_K {
  vertex* V;
  uintE*  coreNumbers;
  intE*   Degrees;
  uintE   k;
  Deg_LessThan_K( vertex* _V, intE* _Degrees, uintE* _coreNumbers,
                  uintE _k )
      : V( _V ), k( _k ), Degrees( _Degrees ), coreNumbers( _coreNumbers )
  {
  }
  inline bool operator()( uintE i ) const
  {
    if ( Degrees[i] < k ) {
      coreNumbers[i] = k - 1;
      Degrees[i]     = 0;
      return true;
    }
    else
      return false;
  }
};

template <class vertex>
struct Deg_AtLeast_K {
  vertex* V;
  intE*   Degrees;
  uintE   k;
  Deg_AtLeast_K( vertex* _V, intE* _Degrees, uintE _k )
      : V( _V ), k( _k ), Degrees( _Degrees )
  {
  }
  inline bool operator()( uintE i ) const { return Degrees[i] >= k; }
};

// assumes symmetric graph

template <class vertex>
void Compute( graph<vertex>& GA, int* results ) {
  const size_t n = GA.n;
  bool* active   = newA( bool, n );
  appl::parallel_for( size_t( 0 ), n, [&]( size_t i ) { active[i] = 1; } );
  vertexSubset Frontier( n, n, active );
  uintE*       coreNumbers = newA( uintE, n );
  intE*        Degrees     = newA( intE, n );

  appl::parallel_for( size_t( 0 ), n, [&]( size_t i ) {
      coreNumbers[i] = 0;
      Degrees[i]     = GA.V[i].getOutDegree();
  } );

  size_t largestCore = -1;
  for ( size_t k = 1; k <= n; k++ ) {
    while ( true ) {
      vertexSubset toRemove = vertexFilter(
          Frontier,
          Deg_LessThan_K<vertex>( GA.V, Degrees, coreNumbers, k ) );
      vertexSubset remaining = vertexFilter(
          Frontier, Deg_AtLeast_K<vertex>( GA.V, Degrees, k ) );
      Frontier.del();
      Frontier = remaining;
      if ( toRemove.numNonzeros() == 0 ) { // fixed point. found k-core
        toRemove.del();
        break;
      } else {
        edgeMap( GA, toRemove, Update_Deg( Degrees ), -1, no_output );
        toRemove.del();
      }
    }
    if ( Frontier.numNonzeros() == 0 ) {
      largestCore = k - 1;
      break;
    }
  }
  bsg_print_int(14850);
  bsg_print_int(largestCore);
  results[0] = largestCore;

  Frontier.del();
}

extern "C" __attribute__ ((noinline))
int kernel_appl_kcore(int* results, symmetricVertex* V, int n, int m, int* dram_buffer) {

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
