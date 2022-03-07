//========================================================================
// Scheduler.h
//========================================================================
// A standard work-stealing scheduler.

#ifndef APPL_WORK_STEALING_SCHEDULER_H
#define APPL_WORK_STEALING_SCHEDULER_H

#include "appl-Task.hpp"
#include "appl-SimpleDeque.hpp"

namespace appl {
namespace local {
extern SimpleDeque<Task*>* g_taskq_p;
}

void spawn( Task* task_p );
void wait( Task* wait_task_p );

} // namespace appl

#include "appl-scheduler.inl"

#endif
