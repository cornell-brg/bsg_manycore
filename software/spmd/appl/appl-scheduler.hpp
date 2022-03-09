//========================================================================
// Scheduler.h
//========================================================================
// A standard work-stealing scheduler.

#ifndef APPL_WORK_STEALING_SCHEDULER_H
#define APPL_WORK_STEALING_SCHEDULER_H

#include "appl-Task.hpp"
#include "appl-SimpleDeque.hpp"
#include "appl-runtime.hpp"

namespace appl {

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

} // namespace appl

#include "appl-scheduler.inl"

#endif
