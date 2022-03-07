#ifndef APPL_CONFIG_GLOBAL_H
#define APPL_CONFIG_GLOBAL_H

#include <cstddef>
#include <stdint.h>

#define MAX_WORKERS (bsg_tiles_X * bsg_tiles_Y)
#define HB_L2_CACHE_LINE_WORDS 16

// per-tile thread local variable to hold fast rand seed
extern int seed;
// parallel for grain size
extern size_t g_pfor_grain_size;

// ref_count stack. This needs to be in the DRAM for AMO
extern int ref_counts[MAX_WORKERS * HB_L2_CACHE_LINE_WORDS] __attribute__ ((section (".dram")));
extern uint32_t ref_count_stack_idx;

#endif
