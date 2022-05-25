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
  while (global::g_stop_flag == 0) {
    appl::sync();
    if (applstatic::local::task != nullptr) {
      applstatic::local::task->execute();
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
