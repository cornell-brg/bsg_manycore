#include "appl-config.hpp"

namespace appl {
namespace local {

int seed = 0;
size_t g_pfor_grain_size = 0;
uint32_t dram_buffer_idx = 0;
int dram_buffer[MAX_WORKERS * BUF_FACTOR * HB_L2_CACHE_LINE_WORDS] __attribute__ ((section (".dram")));

} // namespace local
} // namespace appl
