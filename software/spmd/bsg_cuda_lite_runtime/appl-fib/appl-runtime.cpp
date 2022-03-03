//========================================================================
// runtime.c
//========================================================================

#include "appl-runtime.h"

namespace appl {

void runtime_init( size_t pfor_grain_size = 1 ) {
  // set parallel for grain size
  g_pfor_grain_size = pfor_grain_size;

  // set fast random seed
  seed = __bsg_id;

  return;
}

void runtime_end() {
  return;
}

size_t get_nthreads() {
  // nthreads is simply the tile group size
  return (bsg_tiles_X * bsg_tiles_Y);
}

} // namespace appl
