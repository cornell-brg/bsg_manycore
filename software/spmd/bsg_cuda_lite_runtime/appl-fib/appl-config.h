#ifndef APPL_CONFIG_GLOBAL_H
#define APPL_CONFIG_GLOBAL_H

#include <cstddef>

#define MAX_WORKERS (bsg_tiles_X * bsg_tiles_Y)
#define HB_L2_CACHE_LINE_WORDS 16

// per-tile thread local variable to hold fast rand seed
extern int seed;
// parallel for grain size
extern size_t g_pfor_grain_size;

#endif
