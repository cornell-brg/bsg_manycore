//========================================================================
// Scheduler.cpp
//========================================================================
// A standard work-stealing scheduler.

#include "applrts-scheduler.hpp"

namespace applrts {

void spawn( Task* task_p ) {
#ifdef APPLRTS_DEBUG
  bsg_print_int(12395);
#endif
  local::g_taskq.push_back(task_p);
}

void wait( Task* wait_task_p ) {
  work_stealing_loop(
      [&]() -> bool { return wait_task_p->get_ready_count() <= 1; } );
}

void execute_task( Task* task_p, bool stolen ) {
  while ( task_p ) {
#ifdef APPLRTS_DEBUG
    bsg_print_int(12306);
#endif
    Task* prev_task_p = task_p;
    task_p = task_p->execute();
#ifdef APPLRTS_DEBUG
    bsg_print_int(60321);
#endif

    Task* successor = prev_task_p->get_successor();

    if ( successor ) {
      int old_val = successor->decrement_ready_count();
      if ( old_val == 1 ) {
        spawn( successor );
      }
    }
  }
}

} // namespace applrts

