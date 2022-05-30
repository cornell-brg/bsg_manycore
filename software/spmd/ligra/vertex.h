#ifndef VERTEX_H
#define VERTEX_H

#include "parallel.h"
#include "vertexSubset.h"

namespace decode_uncompressed {

// Used by edgeMapDense. Callers ensure cond(v_id). For each vertex,
// decode its in-edges, and check to see whether this neighbor is in the
// current frontier, calling update if it is. If processing the edges
// sequentially, break once !cond(v_id).
template <class vertex, class F, class G, class VS>
inline void decodeInNghBreakEarly( vertex* v, size_t v_id, VS& vertexSubset,
                                   F& f, G& g, bool parallel = 0 ) {
  uintE d = v->getInDegree();
  if ( !parallel || d < 1000 ) {
    for ( size_t j = 0; j < d; j++ ) {
      uintE ngh = v->getInNeighbor( j );
      if ( vertexSubset.isIn( ngh ) ) {
        auto m = f.update( ngh, v_id );
        g( v_id, m );
      }
      if ( !f.cond( v_id ) )
        break;
    }
  } else {
    appl::parallel_for( uintE( 0 ), d, [&]( uintE j ) {
        uintE ngh = v->getInNeighbor( j );
        if ( vertexSubset.isIn( ngh ) ) {
          auto m = f.updateAtomic( ngh, v_id );
          g( v_id, m );
        }
    } );
  }
}

// Used by edgeMapSparse. For each out-neighbor satisfying cond, call
// updateAtomic.
template <class V, class F, class G>
inline void decodeOutNghSparse( V* v, int i, uintT o, F& f, G& g )
{
  uintE d = v->getOutDegree();
  granular_for( j, 0, d, ( d > 1000 ), {
    uintE ngh = v->getOutNeighbor( j );
    if ( f.cond( ngh ) ) {
      auto m = f.updateAtomic( i, ngh );
      g( ngh, o + j, m );
    }
    else {
      g( ngh, o + j );
    }
  } );
}

} // namespace decode_uncompressed

struct symmetricVertex {
  uintE* neighbors;
  uint32_t pad1;
  uintT degree;
  uint32_t pad2;
  symmetricVertex( uintE* n, uintT d ) : neighbors( n ), degree( d ) {}

  uintE*       getInNeighbors() { return neighbors; }
  const uintE* getInNeighbors() const { return neighbors; }
  uintE*       getOutNeighbors() { return neighbors; }
  const uintE* getOutNeighbors() const { return neighbors; }
  uintE        getInNeighbor( uintT j ) const { return neighbors[j]; }
  uintE        getOutNeighbor( uintT j ) const { return neighbors[j]; }

  void setInNeighbor( uintT j, uintE ngh ) { neighbors[j] = ngh; }
  void setOutNeighbor( uintT j, uintE ngh ) { neighbors[j] = ngh; }
  void setInNeighbors( uintE* _i ) { neighbors = _i; }
  void setOutNeighbors( uintE* _i ) { neighbors = _i; }

  uintT getInDegree() const { return degree; }
  uintT getOutDegree() const { return degree; }
  void  setInDegree( uintT _d ) { degree = _d; }
  void  setOutDegree( uintT _d ) { degree = _d; }
  void  flipEdges() {}

  template <class VS, class F, class G>
  inline void decodeInNghBreakEarly( size_t v_id, VS& vertexSubset, F& f,
                                     G& g, bool parallel = 0 ) {
    decode_uncompressed::decodeInNghBreakEarly<symmetricVertex, F, G, VS>(
        this, v_id, vertexSubset, f, g, parallel );
  }
};



#endif
