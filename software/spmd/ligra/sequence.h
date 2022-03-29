#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "parallel.h"
#include "utils.h"

namespace pbbs {

#define _log_block_size _SCAN_LOG_BSIZE
#define _block_size     ( 1 << _log_block_size )

inline size_t num_blocks( size_t n, size_t block_size )
{
  return ( 1 + ( (n)-1 ) / ( block_size ) );
}

template <class F>
void sliced_for( size_t n, size_t block_size, const F& f )
{
  size_t l = num_blocks( n, block_size );
  appl::parallel_for_1( (size_t)0, l, [&]( size_t i ) {
    size_t s = i * block_size;
    size_t e = (s + block_size) > n ? n : (s + block_size);
    f( i, s, e );
  } );
}

// this will be handled serially as we dont expect too
// many blocks
size_t scan_add(size_t* In, size_t* Out, size_t l) {
  return sequence::plusScan( In, Out, l);
}

template <class Idx_Type>
size_t pack(size_t n, bool* d, Idx_Type* Out) {
  size_t l = num_blocks( n, _block_size );
  size_t* Sums = newA( size_t, l );
  sliced_for( n, _block_size, [&]( size_t i, size_t s, size_t e ) {
      size_t sum = 0;
      for ( size_t idx = s; idx < e; idx++ ) {
        if ( d[idx] ) {
          sum++;
        }
      }
      Sums[i] = sum;
  });
  size_t m = scan_add( Sums, Sums, l );
  sliced_for( n, _block_size, [&]( size_t i, size_t s, size_t e ) {
      Idx_Type* ptr = Out + Sums[i];
      for ( size_t idx = s; idx < e; idx++ ) {
        if( d[idx] ) {
          *ptr = idx;
          ptr++;
        }
      }
  });

  return m;
}

template <class T, class PRED>
size_t filterf( T* In, T* Out, size_t n, PRED p ) {
  size_t l = num_blocks( n, _block_size );
  size_t* Sums = newA( size_t, l );
  sliced_for( n, _block_size, [&]( size_t i, size_t s, size_t e ) {
      size_t sum = 0;
      for ( size_t idx = s; idx < e; idx++ ) {
        if ( p( In[idx] ) ) {
          sum++;
        }
      }
      Sums[i] = sum;
  });
  size_t m = scan_add( Sums, Sums, l );
  sliced_for( n, _block_size, [&]( size_t i, size_t s, size_t e ) {
      T* ptr = Out + Sums[i];
      for ( size_t idx = s; idx < e; idx++ ) {
        if( p( In[idx] ) ) {
          *ptr = idx;
          ptr++;
        }
      }
  });

  return m;
}

} // namespace pbbs
#endif