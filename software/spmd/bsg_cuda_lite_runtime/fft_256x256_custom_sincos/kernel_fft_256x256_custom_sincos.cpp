// 256x256 FFT using four-step method
// In this implementation all tiles write back the intermediate 256-point
// FFT results to the DRAM

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_tile_group_barrier.hpp"

// We need cmath because the current (unoptimized) implementation of
// gather-fft-scatter uses sinf and cosf from newlib. Once we figured
// out a way to eliminate all remote loads due to newlib we can remove
// this dependency.
#include <cmath>

#define CUSTOM_SINCOS (1)
#include "common_fft/common_fft.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> tg_barrier;

FP32Complex fft_workset[NUM_POINTS];

extern "C" __attribute__ ((noinline))
int
kernel_fft_256x256_custom_sincos(FP32Complex *in, FP32Complex *out, int N) {
    /* bsg_set_tile_x_y(); */

    bsg_cuda_print_stat_kernel_start();

    bsg_cuda_print_stat_start(1);
    for (int iter = 0; iter < 2; iter++) {
        load_fft_store(in, in, fft_workset, iter*128+__bsg_id, 256, 256, N, 1);
    }
    bsg_cuda_print_stat_end(1);

    asm volatile("": : :"memory");
    tg_barrier.sync();

    bsg_cuda_print_stat_start(2);
    if (__bsg_id == 0) {
        square_transpose(in, 256);
    }
    bsg_cuda_print_stat_end(2);

    asm volatile("": : :"memory");
    tg_barrier.sync();

    /* debug_print_complex(fft_workset, NUM_POINTS, "After bit reverse"); */

    bsg_cuda_print_stat_start(3);
    for (int iter = 0; iter < 2; iter++) {
        load_fft_store(in, out, fft_workset, iter*128+__bsg_id, 256, 256, N, 0);
    }
    bsg_cuda_print_stat_end(3);

    bsg_cuda_print_stat_kernel_end();

    asm volatile("": : :"memory");
    tg_barrier.sync();

    return 0;
}