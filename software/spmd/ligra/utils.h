#ifndef UTILS_H
#define UTILS_H

#include "appl.hpp"

#if defined(APPL_IMPL_CELLO)
#define newA( __E, __n ) (__E*)cello::arch::malloc( ( __n ) * sizeof( __E ) )
#else
#define newA( __E, __n ) (__E*)appl::appl_malloc( ( __n ) * sizeof( __E ) )
#endif

template <class E>
struct addF {
    E operator()( const E& a, const E& b ) const { return a + b; }
};

template <class ET>
inline void writeAdd(ET* p, ET val) {
  ET result;
  asm volatile ("amoadd.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
}

template <class ET>
inline bool writeMin(ET* p, ET val) {
  ET result;
  asm volatile ("amomin.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
  return result > val;
}

template <class ET>
inline bool writeMinu(ET* p, ET val) {
  ET result;
  asm volatile ("amominu.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
  return result > val;
}

template <class ET>
inline ET min(ET a, ET b) {
  if (a > b) {
    return b;
  } else {
    return a;
  }
}

#define granular_for( _i, _start, _end, _cond, _body )                   \
  {                                                                      \
    if ( _cond ) {                                                       \
      appl::parallel_for( (size_t)_start, (size_t)_end,                  \
                          [&]( size_t _i ) { _body } );                  \
    }                                                                    \
    else {                                                               \
      for ( size_t _i = _start; _i < _end; _i++ ) {                      \
        _body                                                            \
      }                                                                  \
    }                                                                    \
  }

#define _SCAN_LOG_BSIZE 4
#define _SCAN_BSIZE ( 1 << _SCAN_LOG_BSIZE )

namespace sequence {

template <class ET, class intT>
struct getA {
  ET* A;
  getA( ET* AA ) : A( AA ) {}
  ET operator()( intT i ) { return A[i]; }
};

#define nblocks( _n, _bsize ) ( 1 + ( (_n)-1 ) / ( _bsize ) )

#define blocked_for( _i, _s, _e, _bsize, _body )                         \
  {                                                                      \
    intT _ss = _s;                                                       \
    intT _ee = _e;                                                       \
    intT _n  = _ee - _ss;                                                \
    intT _l  = nblocks( _n, _bsize );                                    \
    appl::parallel_for( (intT)0, _l, [&]( intT i ) {                     \
      intT _s = _ss + _i * ( _bsize );                                   \
      intT _e = min( _s + ( _bsize ), _ee );                             \
      _body                                                              \
    } );                                                                 \
  }

template <class OT, class intT, class F, class G>
OT reduceSerial( intT s, intT e, F f, G g )
{
  OT r = g( s );
  for ( intT j = s + 1; j < e; j++ )
    r = f( r, g( j ) );
  return r;
}

template <class ET, class intT, class F, class G>
ET scanSerial( ET* Out, intT s, intT e, F f, G g, ET zero, bool inclusive,
               bool back )
{
  ET r = zero;
  if ( inclusive ) {
    if ( back )
      for ( intT i = e - 1; i >= s; i-- )
        Out[i] = r = f( r, g( i ) );
    else
      for ( intT i = s; i < e; i++ )
        Out[i] = r = f( r, g( i ) );
  }
  else {
    if ( back )
      for ( intT i = e - 1; i >= s; i-- ) {
        ET t   = g( i );
        Out[i] = r;
        r      = f( r, t );
      }
    else
      for ( intT i = s; i < e; i++ ) {
        ET t   = g( i );
        Out[i] = r;
        r      = f( r, t );
      }
  }
  return r;
}

template <class ET, class intT, class F>
ET scanSerial( ET* In, ET* Out, intT n, F f, ET zero )
{
  return scanSerial( Out, (intT)0, n, f, getA<ET, intT>( In ), zero,
                     false, false );
}

// back indicates it runs in reverse direction
template <class ET, class intT, class F, class G>
ET scan( ET* Out, intT s, intT e, F f, G g, ET zero, bool inclusive,
         bool back )
{
  intT n = e - s;
  intT l = nblocks( n, _SCAN_BSIZE );
  if ( l <= 2 )
    return scanSerial( Out, s, e, f, g, zero, inclusive, back );
  ET* Sums = newA( ET, nblocks( n, _SCAN_BSIZE ) );
  blocked_for( i, s, e, _SCAN_BSIZE,
               Sums[i] = reduceSerial<ET>( s, e, f, g ); );
  ET total = scan( Sums, (intT)0, l, f, getA<ET, intT>( Sums ), zero,
                   false, back );
  blocked_for( i, s, e, _SCAN_BSIZE,
               scanSerial( Out, s, e, f, g, Sums[i], inclusive, back ); );
  // free( Sums );
  return total;
}

template <class ET, class intT, class F>
ET scan( ET* In, ET* Out, intT n, F f, ET zero )
{
  return scan( Out, (intT)0, n, f, getA<ET, intT>( In ), zero, false,
               false );
}

template <class ET, class intT>
ET plusScan( ET* In, ET* Out, intT n )
{
  return scan( Out, (intT)0, n, addF<ET>(), getA<ET, intT>( In ), (ET)0,
               false, false );
}

} // namespace sequence
#endif
