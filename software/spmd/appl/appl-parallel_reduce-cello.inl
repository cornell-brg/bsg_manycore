//========================================================================
// parallel_reduce.inl
//========================================================================
#include "cello.hpp"

namespace appl {

template <typename IndexT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( IndexT first, IndexT last, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce ) {
    //return cello::reduce(first, last, 1, initV, func, reduce);
    return initV;
}
} // namespace appl
