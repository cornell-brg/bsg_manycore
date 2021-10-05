// This kernel performs radix of two DIT FFT

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#include "common_fft/common_fft.hpp"

FP32Complex fft_workset[NUM_POINTS];

extern "C" __attribute__ ((noinline))
int
kernel_opt_single_fft(FP32Complex *in, FP32Complex *out, int N) {
    if (bsg_id == 0) {
        bsg_cuda_print_stat_kernel_start();

        /* debug_print_complex(input, NUM_POINTS, "Before bit reverse"); */
        bsg_cuda_print_stat_start(1);
        opt_data_transfer(fft_workset, in, NUM_POINTS);
        bsg_cuda_print_stat_end(1);

        bsg_cuda_print_stat_start(2);
        opt_bit_reverse(fft_workset, NUM_POINTS);
        bsg_cuda_print_stat_end(2);
        /* debug_print_complex(fft_workset, NUM_POINTS, "After bit reverse"); */

        bsg_cuda_print_stat_start(3);
        fft_256(fft_workset);
        bsg_cuda_print_stat_end(3);

        bsg_cuda_print_stat_start(4);
        opt_data_transfer(out, fft_workset, NUM_POINTS);
        bsg_cuda_print_stat_end(4);

        bsg_cuda_print_stat_kernel_end();
        /* debug_print_complex(fft_workset, NUM_POINTS, "Results"); */
    }
    return 0;
}
