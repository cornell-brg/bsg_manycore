#ifndef APPL_PARALLEL_reduce_H
#define APPL_PARALLEL_reduce_H

#include "appl-config.hpp"

namespace appl {

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

} // namespace appl

#ifdef APPL_IMPL_APPLRTS
#include "appl-parallel_reduce-applrts.inl"
#elif defined(APPL_IMPL_CELLO)
#include "appl-parallel_reduce-cello.inl"
#else
#include "appl-parallel_reduce-serial.inl"
#endif

#endif
