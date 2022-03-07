//========================================================================
// runtime.c
//========================================================================

namespace appl {

void runtime_init( SimpleDeque<Task*>* p_taskq_p, size_t pfor_grain_size ) {
  // set parallel for grain size
  local::g_pfor_grain_size = pfor_grain_size;

  // set fast random seed
  local::seed = __bsg_id;

  // alloc per tile task queue
  local::g_taskq_p = p_taskq_p;

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
