//========================================================================
// runtime.h
//========================================================================

#ifndef APPLRTS_GLOBAL_H
#define APPLRTS_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_manycore_atomic.h"
#include "bsg_set_tile_x_y.h"
#include "applrts-config.hpp"
#include "applrts-Task.hpp"
#include "applrts-SimpleDeque.hpp"

namespace applrts {

namespace local {
extern SimpleDeque<Task*> g_taskq;
}

// Initialize the runtime with a default scheduler and a thread pool
void runtime_init( size_t pfor_grain_size = 1 );

// Get number of threads
size_t get_nthreads();

// get current thread id
size_t get_thread_id();

}

#include "applrts-runtime.inl"

#endif
