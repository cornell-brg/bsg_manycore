#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"
#include "ligra.h"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

extern "C" __attribute__ ((noinline))
int kernel_appl_amo_test(int* results, symmetricVertex* V, int n, int m, int* dram_buffer) {

  if (__bsg_id == 0) {
    int* ptr = &(dram_buffer[42]);
    *ptr = 14850;
    int val = 61801;

    writeMin(ptr, val);
    results[0] = bsg_amoadd(ptr, 0);

    writeMax(ptr, val);
    results[1] = bsg_amoadd(ptr, 0);

    uint32_t* uptr = (uint32_t*)(&(dram_buffer[64]));
    uint32_t  uval = -1;
    *uptr = 0;

    writeMinu(uptr, uval);
    results[2] = bsg_amoadd((int*)uptr, 0);

    writeMaxu(uptr, uval);
    results[3] = bsg_amoadd((int*)uptr, 0);

    for (int i = 0; i < 4; i++) {
      bsg_print_int(results[i]);
    }
  }
  barrier.sync();
  return 0;
}
