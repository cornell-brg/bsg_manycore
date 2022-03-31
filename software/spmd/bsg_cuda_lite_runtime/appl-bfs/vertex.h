#ifndef VERTEX_H
#define VERTEX_H

#include "parallel.h"

struct symmetricVertex {
  uintE* neighbors;
  uintT degree;
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
};



#endif
