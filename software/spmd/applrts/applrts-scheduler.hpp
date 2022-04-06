//========================================================================
// Scheduler.h
//========================================================================
// A standard work-stealing scheduler.

#ifndef APPLRTS_WORK_STEALING_SCHEDULER_H
#define APPLRTS_WORK_STEALING_SCHEDULER_H

#include "applrts-Task.hpp"
#include "applrts-SimpleDeque.hpp"
#include "applrts-runtime.hpp"

namespace applrts {

void spawn( Task* task_p );
void wait( Task* wait_task_p );

// Returns a pseudo-random number using the per-thread seed
int fast_rand( void );

// Enter into the work-stealing loop until the condition is met
// cond is a functor returns a bool
template <typename Func>
void work_stealing_loop( Func&& cond );

// Execute the task
void execute_task( Task* task_p, bool stolen );

} // namespace applrts

#include "applrts-scheduler.inl"

#endif
