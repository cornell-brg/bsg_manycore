//========================================================================
// Task.h
//========================================================================
// Task is the base class of task objects with an empty virtual execute()
// function and an atomic ready_count. User can override execute()
// function with custom code.

#ifndef APPL_TASK_H
#define APPL_TASK_H

#include "bsg_manycore.h"
#include "bsg_manycore_atomic.h"
#include "appl-config.h"

// ref_count stack. This needs to be in the DRAM for AMO
extern volatile int ref_counts[MAX_WORKERS * HB_L2_CACHE_LINE_WORDS] __attribute__ ((section (".dram")));
extern uint32_t ref_count_stack_idx;

namespace appl {
class Task {
public:
  Task();

  // Construct with ready_count and a successor, the ready_count of the
  // given successor will be incremented.
  Task( int ready_count, Task* succ_p );

  // Construct with ready_count
  Task( int ready_count );

  // Move constructor is not allowed
  Task( Task&& t ) = delete;

  // Copy is not allowed
  Task( const Task& t ) = delete;

  // execute() function defines the work of a task. It returns a task
  // pointer. After execute() function is called, returned task, if
  // available, will be executed immediately.

  virtual Task* execute();

  virtual ~Task() {}

  // ready_count is generally be used to count the number of tasks that
  // points to this task as successor. If a task's ready_count() is
  // zero, it will be executed soon.

  int get_ready_count();

  void set_ready_count( int ready_count );

  // Set/get the successor task pointer

  void set_successor( Task* task_p );

  Task* get_successor() const;

  // Increment/decrement ready_count by one. They should be atomic when
  // used with a multithreaded scheduler. Should return the value
  // immediately preceding the effects of this function.

  int decrement_ready_count();

  int increment_ready_count();

private:
  // this needs to be a pointer so we can
  // allocate space in DRAM
  int*  m_ready_count_ptr;
  Task* m_successor_ptr;
};

} // namespace appl

#endif
