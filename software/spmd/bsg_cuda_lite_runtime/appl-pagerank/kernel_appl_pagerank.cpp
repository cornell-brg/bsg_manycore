#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"
#include "ligra.h"

float fabs( float x ) {
  if (x > 0) {
    return x;
  } else {
    return -x;
  }
}

template <typename T>
void swap(T& x, T& y) {
  T tmp = x;
  x = y;
  y = tmp;
}

template <class vertex>
struct PR_F {
  float *p_curr, *p_next;
  vertex* V;
  PR_F( float* _p_curr, float* _p_next, vertex* _V )
      : p_curr( _p_curr ), p_next( _p_next ), V( _V )
  {
  }
  inline bool update( uintE s, uintE d )
  { // update function applies PageRank equation
    p_next[d] += p_curr[s] / V[s].getOutDegree();
    return 1;
  }
  inline bool updateAtomic( uintE s, uintE d )
  { // atomic Update
    // writeAdd( &p_next[d], p_curr[s] / V[s].getOutDegree() );
    bsg_print_int(7800);
    return 1;
  }
  inline bool cond( intT d ) { return 1; }
};

// vertex map function to update its p value according to PageRank
// equation
struct PR_Vertex_F {
  float  damping;
  float  addedConstant;
  float* p_curr;
  float* p_next;
  PR_Vertex_F( float* _p_curr, float* _p_next, float _damping, uintE n )
      : p_curr( _p_curr ), p_next( _p_next ), damping( _damping ),
        addedConstant( ( 1 - _damping ) * ( 1 / (float)n ) )
  {
  }
  inline bool operator()( uintE i )
  {
    p_next[i] = damping * p_next[i] + addedConstant;
    return 1;
  }
};

// resets p
struct PR_Vertex_Reset {
  float* p_curr;
  PR_Vertex_Reset( float* _p_curr ) : p_curr( _p_curr ) {}
  inline bool operator()( uintE i )
  {
    p_curr[i] = 0.0;
    return 1;
  }
};

template <class vertex>
void Compute( graph<vertex>& GA, uint32_t maxIters, float* results )
{
  const uintE n       = GA.n;
  const float damping = 0.85, epsilon = 0.0000001;

  float  one_over_n = 1 / (float)n;
  float* p_curr     = newA( float, n );
  float* p_next     = newA( float, n );
  bool* frontier    = newA( bool, n );
  appl::parallel_for( uintE( 0 ), n,
      [&]( uintE i ) {
        p_curr[i]   = one_over_n;
        p_next[i]   = 0;
        frontier[i] = 1;
      }
  );

  vertexSubset Frontier( n, n, frontier );

  uint32_t iter = 0;
  while ( iter++ < maxIters ) {
    edgeMap( GA, Frontier, PR_F<vertex>( p_curr, p_next, GA.V ), 0,
             no_output );
    vertexMap( Frontier, PR_Vertex_F( p_curr, p_next, damping, n ) );
    // compute L1-norm between p_curr and p_next
    {
      appl::parallel_for( uintE( 0 ), n, [&]( uintE i ) {
        p_curr[i] = fabs( p_curr[i] - p_next[i] );
      } );
    }

    //float L1_norm = sequence::plusReduce( p_curr, n );
    float L1_norm = appl::parallel_reduce(uintE(0), n, 0.0f,
        [&](uintE start, uintE end, float initV) {
          float psum = initV;
          for (uintE i = start; i < end; i++) {
            psum += p_curr[i];
          }
          return psum;
        },
        [](float x, float y) { return x + y; }
    );

    bsg_print_int(10086);
    bsg_print_int(iter);
    bsg_print_float(L1_norm);
    if ( L1_norm < epsilon )
      break;

    // reset p_curr
    vertexMap( Frontier, PR_Vertex_Reset( p_curr ) );
    swap( p_curr, p_next );
  }

  // dump
  for (size_t i = 0; i < n; i++) {
    bsg_print_float(p_curr[i]);
    results[i] = p_curr[i];
  }

  Frontier.del();
}

extern "C" __attribute__ ((noinline))
int kernel_appl_pagerank(int* results, symmetricVertex* V, int n, int m, uint32_t maxIters, int* dram_buffer) {

  appl::runtime_init(dram_buffer);
  appl::sync();
  bsg_cuda_print_stat_kernel_start();

  if (__bsg_id == 0) {
    graph<symmetricVertex> G = graph<symmetricVertex>(V, n, m, nullptr);
    Compute(G, maxIters, (float*)(intptr_t)results);
  } else {
    appl::worker_thread_init();
  }

  appl::runtime_end();
  bsg_cuda_print_stat_kernel_end();
  appl::sync();

  return 0;
}
