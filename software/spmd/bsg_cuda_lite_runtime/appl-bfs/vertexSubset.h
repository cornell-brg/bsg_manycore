#ifndef VERTEXSUBSET_H
#define VERTEXSUBSET_H

#include "parallel.h"

struct vertexSubset {
  using S = uintE;

  // An empty vertex set.
  vertexSubset( size_t _n )
      : n( _n ), m( 0 ), d( NULL ), s( NULL ), isDense( 0 )
  {
  }

  /*
  vertexSubset( size_t _n, size_t _m, S* indices )
      : n( _n ), m( _m ), s( indices ), d( NULL ), isDense( 0 )
  {
  }
  */

  vertexSubset( size_t _n, size_t _m, bool* _d )
      : n( _n ), m( _m ), s( NULL ), d( _d ), isDense( 1 )
  {
  }

  vertexSubset( size_t _n, bool* _d )
      : n( _n ), s( NULL ), d( _d ), isDense( 1 )
  {
    // m is the number of TRUE
    // XXX: do a parallel reduce
    size_t sum = 0;
    for (size_t i = 0; i < n; i++) {
      if (d[i]) {
        sum++;
      }
    }
    m = sum;
  }

  // Dense
  inline bool isIn( const uintE& v ) const { return d[v]; }

  size_t size() { return m; }
  size_t numVertices() { return n; }

  size_t numRows() { return n; }
  size_t numNonzeros() { return m; }

  bool isEmpty() { return m == 0; }
  bool dense() { return isDense; }

  /*
  void toDense()
  {
    if ( d == NULL ) {
      // XXX: impl newA
      d = newA( bool, n );
      appl::parallel_for( size_t( 0 ), n, [&]( size_t i ) { d[i] = 0; } );
      appl::parallel_for( size_t( 0 ), m,
                          [&]( size_t i ) { d[s[i]] = 1; } );
    }
    isDense = true;
  }
  */

  S*     s;
  bool*  d;
  size_t n, m;
  bool   isDense;
};

#endif
