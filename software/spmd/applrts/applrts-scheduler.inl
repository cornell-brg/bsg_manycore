//========================================================================
// Scheduler.cpp
//========================================================================

namespace applrts {
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

  // wait until cond() == true
  while ( !cond() ) {
    if (local::seed_task != nullptr) {
      execute_task( local::seed_task, true);
      // reset seed
      // note that it may have already been reset
      local::seed_task = nullptr;
    } else {
      work_stealing_inner_loop();
    }
  } // // while ( !cond() )
}
} // namespace applrts
