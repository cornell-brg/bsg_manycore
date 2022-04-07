//========================================================================
// runtime.inl
//========================================================================
namespace appl {

inline void runtime_init( int* dram_buffer, size_t pfor_grain_size ) {
}

inline void runtime_end() {
}

inline void worker_thread_init() {
}

inline size_t get_nthreads() {
    return 1;
}

inline size_t get_thread_id() {
    return -1;
}

} // namespace appl
