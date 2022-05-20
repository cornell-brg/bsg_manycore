//========================================================================
// parallel_reduce.inl
//========================================================================

#include "applstatic.hpp"

namespace appl {

template <typename IndexT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( IndexT first, IndexT last, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce ) {
  return applstatic::parallel_reduce( first, last, initV, func, reduce);
}

} // namespace appl
