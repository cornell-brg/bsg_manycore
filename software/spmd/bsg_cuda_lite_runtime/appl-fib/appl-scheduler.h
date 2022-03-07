//========================================================================
// Scheduler.h
//========================================================================
// A standard work-stealing scheduler.

#ifndef APPL_WORK_STEALING_SCHEDULER_H
#define APPL_WORK_STEALING_SCHEDULER_H

#include "appl-Task.h"

namespace appl {

void spawn( Task* task_p );
void wait( Task* wait_task_p );

} // namespace appl

#endif
