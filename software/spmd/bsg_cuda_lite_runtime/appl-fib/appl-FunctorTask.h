//========================================================================
// FunctorTask.h
//========================================================================
// FunctorTask is task templated by any type of functor. It also has an
// atomic ready_count.
//
// Helper functions to allocate FunctorTask on the stack or heap are
// provided.
//

#ifndef APPL_FUNCTOR_TASK_H
#define APPL_FUNCTOR_TASK_H

#include <utility>
#include "appl-Task.h"

namespace appl {

//----------------------------------------------------------------------
// FunctorTask definition
//----------------------------------------------------------------------

template <typename Func>
class FunctorTask : public Task {
public:
  // Construct with a functor
  FunctorTask( Func&& func );

  // Construct with a functor and a successor and increment ready_count
  // of the sucessor (if available)
  FunctorTask( Func&& func, Task* succ_p );

  // Move constructor is not allowed
  FunctorTask( FunctorTask&& t ) = delete;

  // Copying is not allowed
  FunctorTask( const FunctorTask& t ) = delete;

  // Executes the functor and returns NULL
  Task* execute();

private:
  Func m_func;
};

//----------------------------------------------------------------------
// mk_task
//----------------------------------------------------------------------
// Helper function to simplify creating tasks from functors.

// Create a FunctorTask on the stack
template <typename Func>
FunctorTask<Func> mk_task( Func&& func )
{
  return FunctorTask<Func>( std::forward<Func>( func ) );
}

// Create a FunctorTask on the stack with a successor
template <typename Func>
FunctorTask<Func> mk_task( Func&& func, Task* successor_p )
{
  return FunctorTask<Func>( std::forward<Func>( func ), successor_p );
}

} // namespace appl

#endif
