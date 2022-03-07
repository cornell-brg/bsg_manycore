#include "appl-config.hpp"

int seed = 0;
size_t g_pfor_grain_size = 1;
int ref_counts[MAX_WORKERS * HB_L2_CACHE_LINE_WORDS] __attribute__ ((section (".dram"))) = {0};
uint32_t ref_count_stack_idx = 0;

