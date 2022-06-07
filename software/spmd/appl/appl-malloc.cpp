#include "appl-malloc.hpp"

namespace appl {

namespace local {

uint32_t dram_buffer_idx = 0;
uint32_t dram_buffer_end = 0;
int* dram_buffer;

} // namespace local

void malloc_init(int* dram_buffer) {
  uint32_t total_cachelines = 128 * BUF_FACTOR;
  uint32_t tile0_cachelines = total_cachelines - 128 * RT_FACTOR;

  local::dram_buffer = &(dram_buffer[__bsg_id * RT_FACTOR * HB_L2_CACHE_LINE_WORDS]);
  local::dram_buffer_end = RT_FACTOR * HB_L2_CACHE_LINE_WORDS;

  if (__bsg_id == 0) {
    local::dram_buffer = &(dram_buffer[128 * RT_FACTOR * HB_L2_CACHE_LINE_WORDS]);
    local::dram_buffer_end = tile0_cachelines * HB_L2_CACHE_LINE_WORDS;
  }
}

} // namespace appl
