//========================================================================
// parallel_invoke.inl
//========================================================================

#include "appl-parallel_invoke.h"
#include "appl-Task.h"

namespace appl {

// Spawn 2, 3, 4, ... functors in parallel

template <typename Func0, typename Func1>
void parallel_invoke( const Func0& func0, const Func1& func1 )
{
  // keep track of pending children
  Task join_point( 1 );

  auto t1 = mk_task( func1, &join_point );

  spawn( &t1 );

  // call func0 directly without spawning
  func0();

  // wait for t1
  wait( &join_point );

  return;
}

} // namespace appl
