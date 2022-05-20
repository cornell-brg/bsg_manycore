//========================================================================
// runtime.inl
//========================================================================

namespace applstatic {

inline void runtime_init( size_t pfor_grain_size ) {
  // reset top level flag
  if (__bsg_id == 0) {
    local::is_top_level = true;
  } else {
    local::is_top_level = false;
  }

  // set parallel for grain size
  local::g_pfor_grain_size = pfor_grain_size;

  return;
}

inline size_t get_nthreads() {
  // nthreads is simply the tile group size
  return (bsg_tiles_X * bsg_tiles_Y);
}

inline size_t get_thread_id() {
  return __bsg_id;
}

} // namespace applstatic
