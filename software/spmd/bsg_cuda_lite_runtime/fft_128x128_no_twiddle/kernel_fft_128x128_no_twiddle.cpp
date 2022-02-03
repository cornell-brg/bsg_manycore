// 128x128 FFT using four-step method
// In this implementation all tiles write back the intermediate 128-point
// FFT results to the DRAM

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#include "bsg_cuda_lite_barrier.h"

/* #include "bsg_tile_group_barrier.hpp" */

#define FFT128
#include "common_fft/common_fft.hpp"

/* bsg_barrier<bsg_tiles_X, bsg_tiles_Y> tg_barrier; */

FP32Complex fft_workset[NUM_POINTS];

extern "C" __attribute__ ((noinline))
int
kernel_fft_128x128_no_twiddle(FP32Complex *in, FP32Complex *out, FP32Complex *tw, int N) {
    /* bsg_set_tile_x_y(); */

    bsg_cuda_print_stat_kernel_start();

    bsg_barrier_hw_tile_group_init();

    bsg_cuda_print_stat_start(1);
    load_fft_store_no_twiddle(in, in, fft_workset, tw, __bsg_id, 128, 128, N, 1);
    bsg_cuda_print_stat_end(1);

    asm volatile("": : :"memory");
    bsg_barrier_hw_tile_group_sync();
    /* tg_barrier.sync(); */

    bsg_cuda_print_stat_start(2);
    opt_square_transpose(in, 128);
    bsg_cuda_print_stat_end(2);

    asm volatile("": : :"memory");
    bsg_barrier_hw_tile_group_sync();
    /* tg_barrier.sync(); */

    bsg_cuda_print_stat_start(3);
    load_fft_store_no_twiddle(in, out, fft_workset, tw, __bsg_id, 128, 128, N, 0);
    bsg_cuda_print_stat_end(3);

    asm volatile("": : :"memory");
    bsg_barrier_hw_tile_group_sync();
    /* tg_barrier.sync(); */

    bsg_cuda_print_stat_kernel_end();

    bsg_fence();

    asm volatile("": : :"memory");
    bsg_barrier_hw_tile_group_sync();
    /* tg_barrier.sync(); */

    return 0;
}
