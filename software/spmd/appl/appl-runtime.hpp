//========================================================================
// runtime.h
//========================================================================

#ifndef APPL_GLOBAL_H
#define APPL_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_manycore_atomic.h"
#include "bsg_set_tile_x_y.h"
#include "appl-config.hpp"
#include "appl-Task.hpp"
#include "appl-SimpleDeque.hpp"

namespace appl {

namespace local {
extern SimpleDeque<Task*>* g_taskq_p;
}

namespace global {
extern int g_stop_flag __attribute__ ((section (".dram")));
}

// Initialize the runtime with a default scheduler and a thread pool
void runtime_init( SimpleDeque<Task*>* p_taskq_p, size_t pfor_grain_size = 1 );

// Terminate runtime by destructing the scheduler and thread pool.
void runtime_end();

// Get number of threads
size_t get_nthreads();

// get current thread id
size_t get_thread_id();

// get remote ptr
template<typename T>
T remote_ptr(T ptr, uint32_t x, uint32_t y);

template<typename T>
T remote_ptr(T ptr, size_t remote_id);

}

#include "appl-runtime.inl"

#endif