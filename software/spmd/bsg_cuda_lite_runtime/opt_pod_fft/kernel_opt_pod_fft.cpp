// This kernel performs radix of two DIT FFT

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define BSG_TILE_GROUP_X_DIM bsg_tiles_X
#define BSG_TILE_GROUP_Y_DIM bsg_tiles_Y
//#define BSG_BARRIER_DEBUG
#include "bsg_tile_group_barrier.h"

INIT_TILE_GROUP_BARRIER(r_barrier, c_barrier, 0, bsg_tiles_X-1, 0, bsg_tiles_Y-1);

#include "common_fft/common_fft.hpp"

FP32Complex fft_workset[NUM_POINTS];

extern "C" __attribute__ ((noinline))
int
kernel_opt_pod_fft(FP32Complex *in, FP32Complex *out, int N) {
    bsg_cuda_print_stat_kernel_start();

    FP32Complex *input  = in+(__bsg_id*NUM_POINTS);
    FP32Complex *output = out+(__bsg_id*NUM_POINTS);

    /* if (__bsg_id == 1) */
    /*     debug_print_complex(input, NUM_POINTS, "Before bit reverse"); */
    bsg_cuda_print_stat_start(1);
    opt_data_transfer(fft_workset, input, NUM_POINTS);
    bsg_cuda_print_stat_end(1);

    bsg_cuda_print_stat_start(2);
    opt_bit_reverse(fft_workset, NUM_POINTS);
    bsg_cuda_print_stat_end(2);
    /* if (__bsg_id == 1) */
    /*     debug_print_complex(fft_workset, NUM_POINTS, "After bit reverse"); */

    bsg_cuda_print_stat_start(3);
    fft_256(fft_workset);
    bsg_cuda_print_stat_end(3);

    bsg_cuda_print_stat_start(4);
    opt_data_transfer(output, fft_workset, NUM_POINTS);
    bsg_cuda_print_stat_end(4);

    bsg_cuda_print_stat_kernel_end();
    /* if (__bsg_id == 1) */
    /*     debug_print_complex(fft_workset, NUM_POINTS, "Results"); */

    bsg_tile_group_barrier(&r_barrier, &c_barrier);

    return 0;
}
