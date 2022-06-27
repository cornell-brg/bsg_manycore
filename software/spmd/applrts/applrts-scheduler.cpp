//========================================================================
// Scheduler.cpp
//========================================================================
// A standard work-stealing scheduler.

#include "applrts-scheduler.hpp"
#include "applrts-stats.hpp"

namespace applrts {

void spawn( Task* task_p ) {
#ifdef APPLRTS_DEBUG
  bsg_print_int(12395);
#endif
  stats::log_task_enqueue();
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

void __attribute__ ((noinline)) work_stealing_inner_loop() {
  size_t my_id = get_thread_id();

  Task* task_p = local::g_taskq.pop_back();
  // execute this task
  if ( task_p ) {
    stats::log_task_dequeue();
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
    stats::log_stealing_attempt();

    if ( task_p ) {
      stats::log_task_stolen();
      // copy task to local
      size_t size = task_p->m_size;
      if (size != -1) {
        char local_task[size];
        char* src = (char*)(intptr_t)task_p;
        for (uint32_t i = 0; i < size; i++) {
          local_task[i] = src[i];
        }
        applrts::Task* local_task_p = (applrts::Task*)(intptr_t)local_task;
        // execute the stolen task copied to local
        // this does not work on reduce
#ifdef APPLRTS_DEBUG
        bsg_print_int(9001);
#endif
        execute_task( local_task_p, true );
      } else {
        // execute the stolen task
#ifdef APPLRTS_DEBUG
        bsg_print_int(9002);
#endif
        execute_task( task_p, true );
      }
    }
  }
}

} // namespace applrts

