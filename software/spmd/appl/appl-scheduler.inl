//========================================================================
// Scheduler.cpp
//========================================================================

namespace appl {
// Linear congruential generator
// https://en.wikipedia.org/wiki/Linear_congruential_generator
//
// Not great in terms of randomness, but should be faster than rand()
inline int fast_rand()
{
  local::seed = ( 214013 * local::seed + 2531011 );
  return ( local::seed >> 16 ) & 0x7FFF;
}

template <typename Func>
inline void work_stealing_loop( Func&& cond ) {
  size_t my_id = get_thread_id();

  // wait until cond() == true
  while ( !cond() ) {
    Task* task_p = local::g_taskq.pop_back();
    // execute this task
    if ( task_p ) {
      execute_task( task_p, false );
    } else {
      // no work on local queue, attempt to steal
      size_t victim_id = fast_rand() % get_nthreads();
      while ( victim_id == my_id ) {
        // pick another victim
        victim_id = fast_rand() % get_nthreads();
      }

      // now found a victim, steal...

      SimpleDeque<Task*>* victim_queue = remote_ptr(&local::g_taskq, victim_id);
      Task* task_p = victim_queue->pop_front();

      if ( task_p ) {
        // execute the stolen task
        execute_task( task_p, true );
      }
    }
  } // // while ( !cond() )
}

} // namespace appl
