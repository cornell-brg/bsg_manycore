//========================================================================
// runtime.h
//========================================================================

#ifndef APPL_GLOBAL_H
#define APPL_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_manycore.h"
#include "appl-config.hpp"
#include "appl-hw-barrier.hpp"

namespace appl {

namespace global {
// global runtime start/stop flag
extern int g_stop_flag __attribute__ ((section (".dram")));
}

// Initialize the runtime with a default scheduler and a thread pool
void runtime_init( int* dram_buffer, size_t pfor_grain_size = 1 );

// Terminate runtime by destructing the scheduler and thread pool.
void runtime_end();

// Worker thread where __bsg_id != 0
void worker_thread_init();

// Get number of threads
size_t get_nthreads();

// get current thread id
size_t get_thread_id();

} // namespace appl

#ifdef APPL_IMPL_APPLRTS
#include "appl-runtime-applrts.inl"
#elif defined(APPL_IMPL_CELLO)
#include "appl-runtime-cello.inl"
#elif defined(APPL_IMPL_STATIC)
#include "appl-runtime-applstatic.inl"
#else
#include "appl-runtime-serial.inl"
#endif

#endif
