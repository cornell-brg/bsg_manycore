#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"
#include "ligra.h"

struct BFS_F {
  uintE* Parents;
  uintE* bfsLvls;
  uintE  lvl;
  BFS_F( uintE* _Parents, uintE* _bfsLvls, uintE  _lvl )
    : Parents( _Parents ), bfsLvls( _bfsLvls ), lvl( _lvl ) {}

  inline bool update( uintE s, uintE d )
  {
    if ( Parents[d] == UINT_E_MAX ) {
      Parents[d] = s;
      bfsLvls[d] = lvl;
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

template <class vertex>
void Compute( graph<vertex>& GA, size_t start, uintE* results ) {
  size_t n     = GA.n;
  uintE* Parents = newA( uintE, n );
  appl::parallel_for( size_t( 0 ), n, [&]( size_t i ) {
      Parents[i] = UINT_E_MAX;
      results[i] = UINT_E_MAX;
      } );
  Parents[start] = start;
  vertexSubset Frontier( n, start ); // creates initial frontier

  uintE lvl = 0;
  while ( !Frontier.isEmpty() ) {    // loop until frontier is empty
    vertexSubset output = edgeMap( GA, Frontier, BFS_F( Parents, results, lvl ) );
    Frontier.del();
    Frontier = output; // set new frontier
    lvl++;
    bsg_print_int(12385);
    bsg_print_int(lvl);
    bsg_print_int(Frontier.numNonzeros());
  }

  Frontier.del();
}

extern "C" __attribute__ ((noinline))
int kernel_appl_bfs(int* results, symmetricVertex* V, int n, int m, int start, int* dram_buffer) {

  appl::runtime_init(dram_buffer);
  appl::sync();
  bsg_cuda_print_stat_kernel_start();

  if (__bsg_id == 0) {
    graph<symmetricVertex> G = graph<symmetricVertex>(V, n, m, nullptr);
    Compute(G, start, (uintE*)(intptr_t)results);
  } else {
    appl::worker_thread_init();
  }

  appl::runtime_end();
  bsg_cuda_print_stat_kernel_end();
  appl::sync();

  return 0;
}
