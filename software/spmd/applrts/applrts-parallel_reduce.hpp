#ifndef APPLRTS_PARALLEL_REDUCE_H
#define APPLRTS_PARALLEL_REDUCE_H

namespace applrts {

//----------------------------------------------------------------------
// parallel_reduce using a body object over a range.
//----------------------------------------------------------------------
// The range is a range object which similar to what is used in
// parallel_for (see parallel_for.h and Range1D.h). The body is an object
// that has the following method.
//
// class BodyT {
// public:
//   // split the body and return a new body
//   BodyT split();
//   // copy constructor
//   BodyT( const BodyT& b );
//   // reduction function, reduce two bodies into one
//   BodyT reduce( BodyT& rhs );
//   // work function, do work over the range and produce a value
//   void operator() ( const Range<T>& range );
// };
template <typename RangeT, typename BodyT>
void parallel_reduce( const RangeT& range, BodyT& body );

//----------------------------------------------------------------------
// Functional form of parallel_reduce
//----------------------------------------------------------------------
template <typename RangeT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( const RangeT& range, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce );

// It performs a parallel reduction by applrtsying func to
// subranges in range and reducing the results using binary operator
// reduction. It returns the result of the reduction.
//
// Types:
//
//   initV  :: ValueT
//   func   :: (IndexT, IndexT, ValueT) -> ValueT
//   reduce :: (ValueT, ValueT) -> ValueT
template <typename IndexT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( IndexT first, IndexT last, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce );
} // namespace applrts

#include "applrts-parallel_reduce.inl"

#endif
