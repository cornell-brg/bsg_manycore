//========================================================================
// parallel_reduce.inl
//========================================================================

#include "applrts.hpp"

namespace appl {

template <typename IndexT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( IndexT first, IndexT last, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce ) {
  return applrts::parallel_reduce( first, last, initV, func, reduce);
}

} // namespace appl
