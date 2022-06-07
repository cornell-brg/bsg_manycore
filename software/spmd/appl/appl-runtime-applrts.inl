//========================================================================
// runtime.inl
//========================================================================

#include "appl-malloc.hpp"
#include "applrts.hpp"

namespace appl {

inline void runtime_init( int* dram_buffer, size_t pfor_grain_size ) {
  appl::config_hw_barrier();
  appl::malloc_init( dram_buffer );
  applrts::runtime_init( pfor_grain_size );
}

inline void runtime_end() {
  if (__bsg_id == 0) {
    for (uint32_t i = 1; i < MAX_WORKERS; i++) {
      int* stop = applrts::remote_ptr(&local::g_stop_flag, i);
      *stop = 1;
    }
  }
}

inline void worker_thread_init() {
  applrts::work_stealing_loop([&]() -> bool {
      return local::g_stop_flag;
      } );
}

inline size_t get_nthreads() {
  return applrts::get_nthreads();
}

inline size_t get_thread_id() {
  return applrts::get_thread_id();
}

} // namespace appl
