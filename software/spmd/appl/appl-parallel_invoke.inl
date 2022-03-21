//========================================================================
// parallel_invoke.inl
//========================================================================

#include "appl-FunctorTask.hpp"
#include "appl-scheduler.hpp"
#include "appl-Task.hpp"

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

template <typename Func0, typename Func1, typename Func2, typename Func3>
void parallel_invoke( const Func0& func0, const Func1& func1,
                      const Func2& func2, const Func3& func3 )
{
  // ready_count of this join point is 1 + number of spawned children
  Task join_point( 1 );

  auto t3 = mk_task( func3, &join_point );
  auto t2 = mk_task( func2, &join_point );
  auto t1 = mk_task( func1, &join_point );

  spawn( &t3 );
  spawn( &t2 );
  spawn( &t1 );

  // call func0 directly without spawning
  func0();

  // wait for t1
  wait( &join_point );

  return;
}

} // namespace appl
