#ifndef APPL_CONFIG_GLOBAL_H
#define APPL_CONFIG_GLOBAL_H

#include <cstddef>

// per-tile thread local variable to hold fast rand seed
extern int seed
// parallel for grain size
extern size_t g_pfor_grain_size = 1;

#endif
