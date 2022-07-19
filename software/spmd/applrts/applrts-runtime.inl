//========================================================================
// runtime.inl
//========================================================================

namespace applrts {

inline void runtime_init( size_t pfor_grain_size ) {
  // set parallel for grain size
  local::g_pfor_grain_size = pfor_grain_size;

  // set fast random seed
  local::seed = __bsg_id;

  // init task queue
  local::g_taskq.reset();

  local::seed_enable = false;

  local::seed_task = nullptr;

  // get seeding target
  size_t next = (get_thread_id() + 1) % get_nthreads();
  if (next == 0) {
    local::seed_target = nullptr;
  } else {
    local::seed_target = applrts::remote_ptr(&local::seed_task, next);
  }
  return;
}

inline size_t get_nthreads() {
  // nthreads is simply the tile group size
  return (bsg_tiles_X * bsg_tiles_Y);
}

inline size_t get_thread_id() {
  return __bsg_id;
}

} // namespace applrts
