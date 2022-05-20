//========================================================================
// parallel_reduce.inl
//========================================================================

#include "applrts-Task.hpp"

namespace applstatic {

//----------------------------------------------------------------------
// ParallelReduceTask
//----------------------------------------------------------------------
// templated class used by parallel_reduce

template <typename IndexT, typename ValueT, typename FuncT>
class ParallelReduceTask : public applrts::Task {
public:
  ParallelReduceTask( const IndexT first, const IndexT last,
                      const ValueT initV, const size_t grain,
                      const FuncT& func, ValueT* pvalues)
      : m_first( first ), m_last( last ), m_initV( initV ),
        m_grain( grain ), m_func( func ), m_pvalues( pvalues )
  {
  }

  Task* execute()
  {
    // calculate per core #iters
    IndexT end = ( m_last - m_first );
    IndexT iters = (end - IndexT( 1 ) ) / (MAX_WORKERS) + IndexT( 1 );
    iters = iters < m_grain ? m_grain : iters;

    // calculate local start and end
    IndexT core_start = __bsg_id * iters;
    IndexT core_end   = (core_start + iters) > end ? end : (core_start + iters);

    // reduce
    ValueT pvalue = m_initV;
    pvalue = m_func( core_start, core_end, m_initV );
    m_pvalues[__bsg_id] = pvalue;
    return nullptr;
  }

private:
  const IndexT  m_first;
  const IndexT  m_last;
  const ValueT  m_initV;
  const size_t  m_grain;
  const FuncT   m_func;
  ValueT*       m_pvalues;
};

template <typename IndexT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( IndexT first, IndexT last, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce )
{
  if ( first < last ) {
    // use the app-specific grain size
    size_t grain = local::g_pfor_grain_size;

    if (local::is_top_level) {
      // partial value buffer
      ValueT* pvalues = (ValueT*)appl::appl_malloc(MAX_WORKERS * sizeof(ValueT));
      // task creation
      ParallelReduceTask<IndexT, ValueT, FuncT> local_task(
          first, last, initV, grain, func, pvalues);
      ParallelReduceTask<IndexT, ValueT, FuncT>* task =
        (ParallelReduceTask<IndexT, ValueT, FuncT>*)appl::appl_malloc(
            sizeof(ParallelReduceTask<IndexT, ValueT, FuncT>));

      // globalization
      char* src = (char*)(intptr_t)&local_task;
      char* dst = (char*)(intptr_t)task;
      for (uint32_t i = 0; i < sizeof(ParallelReduceTask<IndexT, ValueT, FuncT>); i++) {
        dst[i] = src[i];
      }

      // task assignment
      for (uint32_t i = 0; i < MAX_WORKERS; i++) {
        // assign tasks
        applrts::Task** core_task = applrts::remote_ptr(&local::task, i);
        *core_task = task;
      }

      local::is_top_level = false;
      appl::sync();
      local::task->execute();
      appl::sync();
      local::is_top_level = true;

      // do final reduce
      ValueT result = pvalues[0];
      for (uint32_t i = 1; i < MAX_WORKERS; i++) {
        result = reduce(result, pvalues[i]);
      }
      return result;
    } else {
      // nested parallel_reduces are run serially
      return func(first, last, initV);
    } // end of else

  } else {
    return ValueT(-1);
  }
}

} // namespace applstatic
