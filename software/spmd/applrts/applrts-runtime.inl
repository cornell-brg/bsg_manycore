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
  local::g_taskq = (SimpleDeque<Task*>*)appl::appl_malloc(sizeof(SimpleDeque<Task*>));
  local::g_taskq->reset();

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
