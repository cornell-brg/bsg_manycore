//========================================================================
// hw-barrier.h
//========================================================================

#ifndef APPL_HW_BARRIER_H
#define APPL_HW_BARRIER_H

#include "bsg_manycore.h"
#include "bsg_cuda_lite_barrier.h"
#include "bsg_barrier_amoadd.h"

namespace appl {

inline void config_hw_barrier() {
  bsg_barrier_hw_tile_group_init();
}

inline void sync() {
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  bsg_fence();
}

} // namespace appl

#endif
