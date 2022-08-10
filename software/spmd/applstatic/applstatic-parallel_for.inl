//========================================================================
// parallel_for.inl
//========================================================================
#include <string.h>
#include "applstatic-config.hpp"
#include "applstatic-runtime.hpp"
#include "applrts-Task.hpp"
#include "appl-hw-barrier.hpp"
#include "applstatic-config.hpp"

namespace applstatic {

//----------------------------------------------------------------------
// Task definition
//----------------------------------------------------------------------
// templated class used by parallel_for

template <typename IndexT, typename BodyT>
class ParallelForTask : public applrts::Task {
public:
  ParallelForTask( const IndexT first, const IndexT last,
                   const IndexT step, const size_t grain,
                   const BodyT& body )
      : m_first( first ), m_last( last ), m_step( step ),
        m_grain( grain ), m_body( body )
  {
    m_size = sizeof(ParallelForTask<IndexT, BodyT>);
  }

  Task* execute()
  {
    // calculate per core #iters
    IndexT end = ( m_last - m_first - IndexT( 1 ) ) / m_step + IndexT( 1 );
    IndexT iters = (end - IndexT( 1 ) ) / (MAX_WORKERS) + IndexT( 1 );
    iters = iters < m_grain ? m_grain : iters;

    // calculate local start and end
    IndexT core_start = __bsg_id * iters;
    IndexT core_end   = (core_start + iters) > end ? end : (core_start + iters);

    // handle step
    IndexT b = core_start;
    IndexT e = core_end;
    IndexT s = m_step;
    IndexT k = m_first + b * s;

    // run
    for ( IndexT i = b; i < e; ++i, k += s ) {
      m_body( k );
    }
    return nullptr;
  }

private:
  const IndexT m_first;
  const IndexT m_last;
  const IndexT m_step;
  const size_t m_grain;
  const BodyT  m_body;
};

//----------------------------------------------------------------------
// Templated functions
//----------------------------------------------------------------------

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body, size_t grain )
{
  if ( first < last ) {
    // only bsg_id == 0 can have is_top_level set to true
    if (local::is_top_level) {
      // task creation
      ParallelForTask<IndexT, BodyT> local_task(first, last, step, grain, body);
      ParallelForTask<IndexT, BodyT>* task =
        (ParallelForTask<IndexT, BodyT>*)appl::appl_malloc(sizeof(ParallelForTask<IndexT, BodyT>));

      // globalization
      char* src = (char*)(intptr_t)&local_task;
      char* dst = (char*)(intptr_t)task;
      memcpy(dst, src, sizeof(local_task));

      // task assignment
      for (uint32_t i = 1; i < MAX_WORKERS; i++) {
        applrts::Task** core_task = applrts::remote_ptr(&local::task, i);
        *core_task = task;
      }

      local::is_top_level = false;
      appl::sync();
      local_task.execute();
      appl::sync();
      local::is_top_level = true;
    } else {
      // nested parallel_fors are run serially
      for (IndexT i = first; i < last; i += step) {
        body( i );
      }
    } // end of else
  } // end of ( first < last )
}

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body )
{
  // use the app-specific grain size
  size_t grain = get_grain_size(first, last);

  parallel_for( first, last, step, body, grain );
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body )
{
  parallel_for( first, last, step, body, size_t( 1 ) );
}

} // namespace applstatic
