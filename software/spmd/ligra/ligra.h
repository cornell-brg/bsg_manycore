#ifndef LIGRA_H
#define LIGRA_H

#include "parallel.h"
#include "vertex.h"
#include "vertexSubset.h"
#include "graph.h"
#include "utils.h"

typedef uint32_t flags;
const flags      no_output         = 1;
const flags      pack_edges        = 2;
const flags      sparse_no_filter  = 4;
const flags      dense_forward     = 8;
const flags      dense_parallel    = 16;
const flags      remove_duplicates = 32;
inline bool      should_output( const flags& fl )
{
  return !( fl & no_output );
}

auto get_emdense_gen( bool* next ) {
  return [next](uintE id, bool m = false) {
    if (m) {
      next[id] = true;
    }
  };
}

auto get_emdense_nooutput_gen() {
    return [](uintE id, bool m = false) {};
}

template <class vertex, class VS, class F>
vertexSubset edgeMapDense( graph<vertex> GA, VS& vs,
                           F& f, const flags fl ) {
  size_t n  = GA.n;
  vertex* G = GA.V;

  if ( should_output( fl ) ) {
    bool* next = newA( bool, n );
    auto g = get_emdense_gen(next);
    appl::parallel_for( size_t( 0 ), n, [&]( size_t v ) {
        next[v] = 0;
        if ( f.cond( v ) ) {
          G[v].decodeInNghBreakEarly( v, vs, f, g,
                                      fl & dense_parallel );
        }
    } );
    return vertexSubset( n, next );
  } else {
    auto g = get_emdense_nooutput_gen();
    appl::parallel_for( size_t( 0 ), n, [&]( size_t v ) {
        if ( f.cond( v ) ) {
          G[v].decodeInNghBreakEarly( v, vs, f, g,
                                      fl & dense_parallel );
        }
    } );
    // no output so empty subset
    return vertexSubset( n );
  }
}

// Regular edgeMap, where no extra data is stored per vertex.
template <class vertex, class VS, class F>
vertexSubset edgeMap( graph<vertex> GA, VS& vs, F f, intT threshold = -1,
                      const flags& fl = 0 ) {
  size_t numVertices = GA.n;
  size_t numEdges = GA.m;
  size_t m = vs.numNonzeros();

  if ( threshold == -1 ) {
    threshold = numEdges / 20; // default threshold
  }

  vertex* G = GA.V;
  if ( numVertices != vs.numRows() ) {
    // error: edgeMap: Sizes Don't match
    bsg_print_int(7700);
  }

  // early return if vs is empty
  if ( vs.size() == 0 ) {
    return vertexSubset( numVertices );
  }

  // only do dense
  vs.toDense();
  return edgeMapDense<vertex, VS, F>( GA, vs, f, fl );
}

//*****VERTEX FUNCTIONS*****

template <class VS, class F>
void vertexMap( VS& vs, F f ) {
  if (vs.isDense()) {
    size_t n = vs.numRows();
    appl::parallel_for( size_t( 0 ), n, [&]( size_t i ) {
        if (vs.isIn(i)) {
          f(i);
        }
    } );
  } else {
    size_t m = vs.numNonzeros();
    appl::parallel_for( size_t( 0 ), m,
        [&]( size_t i ) { f( vs.vtx( i ) ); } );
  }
}

template <class VS, class F>
vertexSubset vertexFilter( VS& vs, F f ) {
  size_t n = vs.numRows();
  vs.toDense();
  bool* next = newA( bool, n );
  appl::parallel_for( size_t( 0 ), n, [&]( size_t i ) {
      if (vs.isIn(i)) {
        next[i] = f(i);
      }
  } );
  return vertexSubset( n, next );
}

#endif
