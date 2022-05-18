#include "appl-malloc.hpp"

namespace appl {

namespace local {

uint32_t dram_buffer_idx = 0;
int* dram_buffer;

} // namespace local

void malloc_init(int* dram_buffer) {
  local::dram_buffer = &(dram_buffer[__bsg_id * BUF_FACTOR * HB_L2_CACHE_LINE_WORDS]);
}

} // namespace appl
