//========================================================================
// runtime.inl
//========================================================================

namespace applrts {

inline void runtime_init( int* dram_buffer, size_t pfor_grain_size ) {
  // set parallel for grain size
  local::g_pfor_grain_size = pfor_grain_size;

  // set fast random seed
  local::seed = __bsg_id;

  // set global buffer
  local::dram_buffer = &(dram_buffer[__bsg_id * BUF_FACTOR * HB_L2_CACHE_LINE_WORDS]);

  // init task queue
  local::g_taskq.reset();

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
