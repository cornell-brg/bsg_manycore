//========================================================================
// runtime.inl
//========================================================================

#include "appl-malloc.hpp"
#include "applstatic.hpp"

namespace appl {

inline void runtime_init( int* dram_buffer, size_t pfor_grain_size ) {
  appl::malloc_init( dram_buffer );
  applstatic::runtime_init( pfor_grain_size );
  config_hw_barrier();
}

inline void runtime_end() {
  if (__bsg_id == 0) {
    appl::sync();
    if (__bsg_id == 0) {
      for (uint32_t i = 1; i < MAX_WORKERS; i++) {
        int* stop = applrts::remote_ptr(&global::g_stop_flag, i);
        *stop = 1;
      }
    }
    appl::sync();
  }
}

inline void worker_thread_init() {
  while (local::g_stop_flag == 0) {
    appl::sync();
    if (applstatic::local::task != nullptr) {
      size_t size = applstatic::local::task->m_size;
      char local_task[size];
      char* src = (char*)(intptr_t)applstatic::local::task;
      for (uint32_t i = 0; i < size; i++) {
        local_task[i] = src[i];
      }
      applrts::Task* local_task_p = (applrts::Task*)(intptr_t)local_task;
      local_task_p->execute();
      applstatic::local::task = nullptr;
    }
    appl::sync();
  }
}

inline size_t get_nthreads() {
  return applstatic::get_nthreads();
}

inline size_t get_thread_id() {
  return applstatic::get_thread_id();
}

} // namespace appl
