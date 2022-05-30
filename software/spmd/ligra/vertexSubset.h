#ifndef VERTEXSUBSET_H
#define VERTEXSUBSET_H

#include "parallel.h"
#include "utils.h"
#include "sequence.h"

struct vertexSubset {
  using S = uintE;

  // An empty vertex set.
  vertexSubset( size_t _n )
      : n( _n ), m( 0 ), d( NULL ), s( NULL ), dense( 0 )
  {
  }

  // A vertexSubset with a single vertex.
  vertexSubset( size_t _n, uintE v )
      : n( _n ), m( 1 ), d( NULL ), dense( 0 )
  {
    s = newA( uintE, 1 );
    s[0] = v;
  }

  vertexSubset( size_t _n, size_t _m, S* indices )
      : n( _n ), m( _m ), s( indices ), d( NULL ), dense( 0 )
  {
  }

  vertexSubset( size_t _n, size_t _m, bool* _d )
      : n( _n ), m( _m ), s( NULL ), d( _d ), dense( 1 )
  {
  }

  vertexSubset( size_t _n, bool* _d )
      : n( _n ), s( NULL ), d( _d ), dense( 1 )
  {
    m = appl::parallel_reduce(size_t(0), n, size_t(0),
        [&](size_t start, size_t end, size_t initV) {
          size_t psum = initV;
          for (size_t i = start; i < end; i++) {
            if (d[i]) {
              psum++;
            }
          }
          return psum;
        },
        [](size_t x, size_t y) { return x + y; }
      );
    // m is the number of TRUE
  }

  void del() { }

  inline bool   isIn( const uintE& v ) const { return d[v]; }
  inline uintE& vtx( const uintE& i ) const { return s[i]; }

  size_t size() { return m; }
  size_t numVertices() { return n; }

  size_t numRows() { return n; }
  size_t numNonzeros() { return m; }

  bool isEmpty() { return m == 0; }
  bool isDense() { return dense; }

  void toDense()
  {
    if ( d == NULL ) {
      d = newA( bool, n );
      appl::parallel_for( size_t( 0 ), n, [&]( size_t i ) { d[i] = 0; } );
      appl::parallel_for( size_t( 0 ), m,
                          [&]( size_t i ) { d[s[i]] = 1; } );
    }
    dense = true;
  }

  void toSparse()
  {
    if ( s == NULL && m > 0 ) {
      s = new(S, m);
      size_t sparse_m = pack(d, s);
      if ( sparse_m != m ) {
        // ERROR bad stored value of m
        bsg_print_int(7700);
      }
    }
    isDense = false;
  }

  S*     s;
  bool*  d;
  size_t n, m;
  bool   dense;
};

#endif
