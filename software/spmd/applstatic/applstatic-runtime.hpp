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
#include "applstatic-config.hpp"

#include "applrts-Task.hpp"

namespace applstatic {

namespace local {
// statically assigned task
extern applrts::Task* task;
}

// Initialize the runtime with a default scheduler and a thread pool
void runtime_init( size_t pfor_grain_size = 0 );

// Get number of threads
size_t get_nthreads();

// get current thread id
size_t get_thread_id();

}

#include "applstatic-runtime.inl"

#endif
