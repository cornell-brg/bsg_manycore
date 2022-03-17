#ifndef APPL_CONFIG_GLOBAL_H
#define APPL_CONFIG_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define MAX_WORKERS (bsg_tiles_X * bsg_tiles_Y)
#define HB_L2_CACHE_LINE_WORDS 16
#define BUF_FACTOR 129

// remote pointer calculation
#define GROUP_EPA_WIDTH 18
#define GROUP_X_CORD_WIDTH 6
#define GROUP_Y_CORD_WIDTH 5
#define GROUP_X_CORD_SHIFT (GROUP_EPA_WIDTH)
#define GROUP_Y_CORD_SHIFT (GROUP_X_CORD_SHIFT+GROUP_X_CORD_WIDTH)
#define GROUP_PREFIX_SHIFT (GROUP_Y_CORD_SHIFT+GROUP_Y_CORD_WIDTH)

// utils
namespace appl {
namespace local {

// per-tile thread local variable to hold fast rand seed
extern int seed;
// parallel for grain size
extern size_t g_pfor_grain_size;

// ref_count stack. This needs to be in the DRAM for AMO
extern uint32_t dram_buffer_idx;

} // namespace local

namespace global {

extern int dram_buffer[MAX_WORKERS * BUF_FACTOR * HB_L2_CACHE_LINE_WORDS] __attribute__ ((section (".dram")));

} // namespace global

// linear allocator in DRAM
inline int* brg_malloc() {
  int* val = &(global::dram_buffer[__bsg_id * BUF_FACTOR * HB_L2_CACHE_LINE_WORDS + local::dram_buffer_idx++]);
  bsg_print_int((intptr_t)val);
  bsg_print_int(local::dram_buffer_idx);
  return val;
}

template<typename T>
inline T remote_ptr(T ptr, uint32_t x, uint32_t y) {
  unsigned int local_ptr = ((1 << GROUP_EPA_WIDTH) - 1) & ((unsigned int) ptr);
  T r_ptr = (T)( ((1 << GROUP_PREFIX_SHIFT)
                    | (y << GROUP_Y_CORD_SHIFT)
                    | (x << GROUP_X_CORD_SHIFT)
                    | (local_ptr)));
  return r_ptr;
}

template<typename T>
inline T remote_ptr(T ptr, size_t remote_id) {
  uint32_t remote_x = remote_id % bsg_tiles_X;
  uint32_t remote_y = remote_id / bsg_tiles_X;
  return remote_ptr<T>(ptr, remote_x, remote_y);
}

} // namespace appl

#endif
