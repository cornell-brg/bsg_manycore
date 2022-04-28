#ifndef APPLRTS_CONFIG_GLOBAL_H
#define APPLRTS_CONFIG_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define MAX_WORKERS (bsg_tiles_X * bsg_tiles_Y)
#define HB_L2_CACHE_LINE_WORDS 16
#define BUF_FACTOR 2049

// remote pointer calculation
#define GROUP_EPA_WIDTH 18
#define GROUP_X_CORD_WIDTH 6
#define GROUP_Y_CORD_WIDTH 5
#define GROUP_X_CORD_SHIFT (GROUP_EPA_WIDTH)
#define GROUP_Y_CORD_SHIFT (GROUP_X_CORD_SHIFT+GROUP_X_CORD_WIDTH)
#define GROUP_PREFIX_SHIFT (GROUP_Y_CORD_SHIFT+GROUP_Y_CORD_WIDTH)

#undef APPLRTS_DEBUG

// utils
namespace applrts {
namespace local {

// per-tile thread local variable to hold fast rand seed
extern int seed;
// parallel for grain size
extern size_t g_pfor_grain_size;

// ref_count stack. This needs to be in the DRAM for AMO
extern uint32_t dram_buffer_idx;

extern int* dram_buffer;

} // namespace local

// linear allocator in DRAM
inline int* brg_malloc() {
  int* val = &(local::dram_buffer[local::dram_buffer_idx++]);
#ifdef APPLRTS_DEBUG
  bsg_print_hexadecimal((intptr_t)val);
  bsg_print_int(local::dram_buffer_idx);
#endif
  if (local::dram_buffer_idx > HB_L2_CACHE_LINE_WORDS * BUF_FACTOR) {
    bsg_print_int(7600);
  }
  return val;
}

// number of bytes
inline void* brg_malloc(uint32_t size) {
  uint32_t size_4 = size >> 2;
  if (size & 0x3 != 0) {
    size_4++;
  }
  void* val = (void*)(&(local::dram_buffer[local::dram_buffer_idx]));
  local::dram_buffer_idx += size_4;
#ifdef APPLRTS_DEBUG
  bsg_print_hexadecimal((intptr_t)val);
  bsg_print_int(local::dram_buffer_idx);
#endif
  if (local::dram_buffer_idx > HB_L2_CACHE_LINE_WORDS * BUF_FACTOR) {
    bsg_print_int(7600);
  }
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

} // namespace applrts

#endif
