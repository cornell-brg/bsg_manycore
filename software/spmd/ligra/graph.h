#ifndef GRAPH_H
#define GRAPH_H
#include "parallel.h"
#include "vertex.h"

typedef void Deletable;

template <class vertex>
struct graph {
  vertex* V;
  int     n;
  int     m;
  bool    transposed;
  uintE*  flags;
  void*   D;

  graph( vertex* _V, int _n, int _m, Deletable* _D )
      : V( _V ), n( _n ), m( _m ), D( _D ), flags( NULL ), transposed( 0 )
  {
  }

  graph( vertex* _V, int _n, int _m, Deletable* _D, uintE* _flags )
      : V( _V ), n( _n ), m( _m ), D( _D ), flags( _flags ),
        transposed( 0 )
  {
  }

  void transpose() {}
};

#endif
