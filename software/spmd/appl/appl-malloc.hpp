//========================================================================
// appl-malloc.h
//========================================================================

#ifndef APPL_MALLOC_GLOBAL_H
#define APPL_MALLOC_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define HB_L2_CACHE_LINE_WORDS 16
#define BUF_FACTOR 16385
#define RT_FACTOR 2049

#undef MALLOC_DEBUG

// utils
namespace appl {
namespace local {

// ref_count stack. This needs to be in the DRAM for AMO
extern uint32_t dram_buffer_idx;
extern uint32_t dram_buffer_end;

extern int* dram_buffer;

} // namespace local

void malloc_init(int* dram_buffer);

// linear allocator in DRAM
inline int* appl_malloc() {
  int* val = &(local::dram_buffer[local::dram_buffer_idx++]);
#ifdef MALLOC_DEBUG
  bsg_print_hexadecimal((intptr_t)val);
  bsg_print_int(local::dram_buffer_idx);
#endif
  if (local::dram_buffer_idx > local::dram_buffer_end) {
    bsg_print_int(7600);
  }
  return val;
}

// number of bytes
inline void* appl_malloc(uint32_t size) {
  uint32_t size_4 = size >> 2;
  if (size & 0x3 != 0) {
    size_4++;
  }
  void* val = (void*)(&(local::dram_buffer[local::dram_buffer_idx]));
  local::dram_buffer_idx += size_4;
#ifdef MALLOC_DEBUG
  bsg_print_hexadecimal((intptr_t)val);
  bsg_print_int(local::dram_buffer_idx);
#endif
  if (local::dram_buffer_idx > local::dram_buffer_end) {
    bsg_print_int(7600);
  }
  return val;
}

} // namespace appl

#endif
