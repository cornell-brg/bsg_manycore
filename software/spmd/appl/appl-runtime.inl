//========================================================================
// runtime.inl
//========================================================================

namespace appl {

inline void runtime_init( SimpleDeque<Task*>* p_taskq_p, size_t pfor_grain_size ) {
  // set parallel for grain size
  local::g_pfor_grain_size = pfor_grain_size;

  // set fast random seed
  local::seed = __bsg_id;

  // alloc per tile task queue
  local::g_taskq_p = p_taskq_p;

  return;
}

inline void runtime_end() {
  // this does not order any computation
  bsg_amoswap(&global::g_stop_flag, 1);
  return;
}

inline size_t get_nthreads() {
  // nthreads is simply the tile group size
  return (bsg_tiles_X * bsg_tiles_Y);
}

inline size_t get_thread_id() {
  return __bsg_id;
}

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

} // namespace appl
