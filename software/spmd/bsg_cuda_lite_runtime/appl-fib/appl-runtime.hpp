//========================================================================
// runtime.h
//========================================================================

#ifndef APPL_GLOBAL_H
#define APPL_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "appl-config.hpp"

namespace appl {

// Initialize the runtime with a default scheduler and a thread pool
void runtime_init( size_t pfor_grain_size = 1 );

// Terminate runtime by destructing the scheduler and thread pool.
void runtime_end();

// Get number of threads
size_t get_nthreads();

}

#include "appl-runtime.inl"

#endif
