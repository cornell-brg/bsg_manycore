#ifndef APPLSTATIC_PARALLEL_REDUCE_H
#define APPLSTATIC_PARALLEL_REDUCE_H

namespace applrts {

//----------------------------------------------------------------------
// Functional form of parallel_reduce
//----------------------------------------------------------------------
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

#include "applstatic-parallel_reduce.inl"

#endif
