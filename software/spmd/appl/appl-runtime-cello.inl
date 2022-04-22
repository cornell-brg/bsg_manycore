//========================================================================
// runtime.inl
//========================================================================
#include "limoncello.hpp"

namespace appl {

/**
 * Is set to 1 when worker threads should return
 */
__attribute__((weak, section(".dram")))
int __cello_stop = 0;

/**
 * Called by all threads.
 */
inline void runtime_init( int* dram_buffer, size_t pfor_grain_size ) {
    using namespace cello;
    using namespace arch;
    if (get_thread_id()==0) {
        __cello_arch_dram_heap_ptr = reinterpret_cast<intptr_t>(dram_buffer);
        cello::arch::work_queue::Instance()->clear();
    }
}

/**
 * Called by all threads.
 */
inline void runtime_end() {
    if (get_thread_id() == 0) {
        __cello_stop = 1;
    }
}

/**
 * This function is called by all-but-one thread.
 */
inline void worker_thread_init() {
    using namespace cello;
    using namespace arch;
    while (__cello_stop != 1) { cello::arch::worker::LocalWorker()->work(); }
}    

inline size_t get_nthreads() {
    return (bsg_tiles_X*bsg_tiles_Y);
}

inline size_t get_thread_id() {
    using namespace cello;
    using namespace arch;
    return worker::LocalWorker()->id();
}

} // namespace appl
