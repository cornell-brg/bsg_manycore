//========================================================================
// parallel_reduce.inl
//========================================================================

namespace appl {

template <typename IndexT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( IndexT first, IndexT last, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce ) {
  ValueT result = initV;
  for (IndexT i = first; i < last; i++) {
    result = reduce(result, func(i, i+1, initV));
  }
  return result;
}

} // namespace appl
