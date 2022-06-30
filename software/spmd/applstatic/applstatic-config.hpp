#ifndef APPLSTATIC_CONFIG_GLOBAL_H
#define APPLSTATIC_CONFIG_GLOBAL_H

#include <cstddef>
#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define MAX_WORKERS (bsg_tiles_X * bsg_tiles_Y)

// remote pointer calculation
#define GROUP_EPA_WIDTH 18
#define GROUP_X_CORD_WIDTH 6
#define GROUP_Y_CORD_WIDTH 5
#define GROUP_X_CORD_SHIFT (GROUP_EPA_WIDTH)
#define GROUP_Y_CORD_SHIFT (GROUP_X_CORD_SHIFT+GROUP_X_CORD_WIDTH)
#define GROUP_PREFIX_SHIFT (GROUP_Y_CORD_SHIFT+GROUP_Y_CORD_WIDTH)

#undef APPLSTATIC_DEBUG

// utils
namespace applstatic {
namespace local {

// parallel for grain size
extern size_t g_pfor_grain_size;

// top level flag
extern bool is_top_level;

} // namespace local

/*
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
*/

inline size_t get_grain_size(size_t first, size_t last) {
  size_t grain = local::g_pfor_grain_size;
  if (grain == 0) {
    grain = (last - first) / (8 * MAX_WORKERS);
    if (grain > 2048) {
      grain = 2048;
    }
    if (grain == 0) {
      grain = 1;
    }
  }
  return grain;
}

} // namespace applstatic

#endif
