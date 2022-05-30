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

auto get_emsparse_gen( uintE* outEdges ) {
    return [outEdges]( uintE ngh, uintT offset, bool m = false ) {
      if ( m ) {
        outEdges[offset] = ngh;
      } else {
        outEdges[offset] = UINT_E_MAX;
      }
    };
}

auto get_emsparse_nooutput_gen() {
    return [&]( uintE ngh, uintT offset, bool m = false ) {};
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

template <class vertex, class VS, class F>
vertexSubset edgeMapSparse( graph<vertex> GA, vertex* frontierVertices, VS& indices,
                            uintT* degrees, uintT m, F& f, const flags fl ) {
  using S = uintE;
  int n  = indices.n;
  S*   outEdges;
  int outEdgeCount = 0;

  if ( should_output( fl ) ) {
    uintT* offsets = degrees;
    outEdgeCount   = sequence::plusScan( offsets, offsets, m );
    outEdges       = newA( S, outEdgeCount );
    auto g         = get_emsparse_gen( outEdges );
    appl::parallel_for( uintT( 0 ), m, [&]( uintT i ) {
      uintT  v = indices.vtx( i ), o = offsets[i];
      vertex vert = frontierVertices[i];
      vert.decodeOutNghSparse( v, o, f, g );
    } );
  }
  else {
    auto g = get_emsparse_nooutput_gen();
    appl::parallel_for( uintT( 0 ), m, [&]( uintT i ) {
      uintT  v    = indices.vtx( i );
      vertex vert = frontierVertices[i];
      vert.decodeOutNghSparse( v, 0, f, g );
    } );
  }

  if ( should_output( fl ) ) {
    S* nextIndices = newA( S, outEdgeCount );
    auto p = []( uintE& v ) {
      return v != UINT_E_MAX;
    };
    size_t nextM =
        pbbs::filterf( outEdges, nextIndices, outEdgeCount, p );
    // free( outEdges );
    return vertexSubset( n, nextM, nextIndices );
  } else {
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

  vs.toSparse();
  uintT*  degrees          = newA( uintT, m );
  vertex* frontierVertices = newA( vertex, m );

  appl::parallel_for( size_t( 0 ), m, [&]( int i ) {
      uintE  v_id         = vs.vtx( i );
      vertex v            = G[v_id];
      degrees[i]          = v.getOutDegree();
      frontierVertices[i] = v;
  });

  bsg_print_int(90189);
  for (size_t i = 0; i < m; i++) {
    bsg_print_int(degrees[i]);
  }

  //uintT outDegrees = sequence::plusReduce( degrees, m );
  uintT outDegrees = appl::parallel_reduce(size_t(0), m, size_t(0),
      [&](size_t start, size_t end, size_t initV) {
        size_t psum = initV;
        for (size_t i = start; i < end; i++) {
          psum += degrees[i];
        }
        return psum;
      },
      [](size_t x, size_t y) { return x + y; }
  );

  if ( outDegrees == 0 ) {
    return vertexSubset( numVertices );
  }

  bsg_print_int(10086);
  bsg_print_int(outDegrees);

  if ( m + outDegrees > threshold ) {
    bsg_print_int(10087);
    vs.toDense();
    return edgeMapDense<vertex, VS, F>( GA, vs, f, fl );
  } else {
    bsg_print_int(10088);
    auto vs_out = edgeMapSparse<vertex, VS, F>(
                    GA, frontierVertices, vs, degrees,
                    vs.numNonzeros(), f, fl );
    return vs_out;
  }
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

// cond function that always returns true
inline bool cond_true( intT d )
{
  return 1;
}

#endif
