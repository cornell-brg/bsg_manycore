//========================================================================
// runtime.inl
//========================================================================
#include "cello.hpp"
#include "cello_scheduler.h"
#include "cello_malloc.h"

namespace appl {

/**
 * Is set to 1 when worker threads should return
 */
__attribute__((weak, section(".dram")))
i32 __cello_stop_flag = 1;

/**
 * Called by all threads.
 */
inline void runtime_init( int* dram_buffer, size_t pfor_grain_size ) {
    appl::config_hw_barrier();
    appl::sync();
    cello_scheduler_init();
    if (get_thread_id()==0) {
        __cello_malloc_heap_ptr = reinterpret_cast<intptr_t>(dram_buffer);
    }
    appl::sync();
}

/**
 * Called by all threads.
 */
inline void runtime_end() {
    if (get_thread_id() == 0) {
        __cello_stop_flag = 0;
    }
}

/**
 * This function is called by all-but-one thread.
 */
inline void worker_thread_init() {
    cello_scheduler(&__cello_stop_flag);
}    

inline size_t get_nthreads() {
    return (bsg_tiles_X*bsg_tiles_Y);
}

inline size_t get_thread_id() {
    return __bsg_id;
}

} // namespace appl
