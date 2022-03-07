#ifndef APPL_CONFIG_GLOBAL_H
#define APPL_CONFIG_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_set_tile_x_y.h"

#define MAX_WORKERS (bsg_tiles_X * bsg_tiles_Y)
#define HB_L2_CACHE_LINE_WORDS 16
#define BUF_FACTOR 33

// per-tile thread local variable to hold fast rand seed
extern int seed;
// parallel for grain size
extern size_t g_pfor_grain_size;

// ref_count stack. This needs to be in the DRAM for AMO
extern uint32_t dram_buffer_idx;
extern int dram_buffer[MAX_WORKERS * BUF_FACTOR * HB_L2_CACHE_LINE_WORDS] __attribute__ ((section (".dram")));

// utils
namespace appl {

  // linear allocator in DRAM
  inline int* brg_malloc() {
    return &(dram_buffer[__bsg_id * BUF_FACTOR * HB_L2_CACHE_LINE_WORDS + dram_buffer_idx++]);
  }

} // namespace appl

#endif
