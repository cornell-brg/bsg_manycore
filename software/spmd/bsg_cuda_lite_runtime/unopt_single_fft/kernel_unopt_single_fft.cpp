// This kernel performs radix of two DIT FFT

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#include <cmath>
#include <complex>

#define NUM_POINTS 256

typedef std::complex<float> FP32Complex;
typedef volatile FP32Complex *bsg_remote_complex_ptr;

FP32Complex *input;
FP32Complex *output;
FP32Complex fft_workset[NUM_POINTS];

#define bsg_remote_complex(x, y, local_addr) ((bsg_remote_complex_ptr)\
        (( REMOTE_EPA_PREFIX << REMOTE_EPA_MASK_SHIFTS )\
           | ((y) << Y_CORD_SHIFTS) \
           | ((x) << X_CORD_SHIFTS) \
           | ((int) (local_addr))   \
           )\
        )

#define bsg_complex_store(x,y,local_addr,val) do { *(bsg_remote_complex((x),(y),(local_addr))) = (FP32Complex) (val); } while(0)
#define bsg_complex_load(x,y,local_addr,val) do { val = *(bsg_remote_complex((x),(y),(local_addr))); } while(0)

int workset_idx = 0;

void
fft_swizzle(int start, int stride) {
    FP32Complex val;
    if (NUM_POINTS > stride) {
        fft_swizzle(start, stride*2);
        fft_swizzle(start+stride, stride*2);
    } else {
        val = input[start];
        fft_workset[workset_idx] = val;
        workset_idx += 1;
    }
}

void
fft() {
    int even_idx, odd_idx, n = 2;
    float k_div_n;
    FP32Complex exp_val, tw_val, even_val, odd_val;
    float PI = std::acos(-1);
    FP32Complex ImagURoot = FP32Complex(0.0, 1.0);

    while (n <= NUM_POINTS) {
        for (int i = 0; i < NUM_POINTS; i += n) {
            for (int k = 0; k < n/2; k++) {
                even_idx = i+k;
                odd_idx  = even_idx + n/2;

                k_div_n = (float) k / (float) n;
                exp_val = -2.0f*ImagURoot*PI*k_div_n;

                even_val = fft_workset[even_idx];
                odd_val  = fft_workset[odd_idx];

                tw_val = std::exp(exp_val)*odd_val;

                fft_workset[even_idx] = even_val + tw_val;
                fft_workset[odd_idx]  = even_val - tw_val;
            }
        }
        n = n * 2;
    }
}

extern "C" __attribute__ ((noinline))
int
kernel_unopt_single_fft(FP32Complex *in, FP32Complex *out, int N) {
    if (bsg_id == 0) {
        bsg_cuda_print_stat_kernel_start();
        input = in;
        output = out;
        /* bsg_printf("Before swizzle:\n"); */
        /* for(int i = 0; i < NUM_POINTS; i++) { */
        /*     float rr = input[i].real(); */
        /*     float ii = input[i].imag(); */
        /*     bsg_printf("(%08X)+(%08X)i ", *(int*)&rr, *(int*)&ii); */
        /* } */

        bsg_cuda_print_stat_start(1);
        fft_swizzle(0, 1);
        bsg_cuda_print_stat_end(1);

        /* bsg_printf("\nAfter swizzle:\n"); */
        /* for(int i = 0; i < NUM_POINTS; i++) { */
        /*     float rr = fft_workset[i].real(); */
        /*     float ii = fft_workset[i].imag(); */
        /*     bsg_printf("(%08X)+(%08X)i ", *(int*)&rr, *(int*)&ii); */
        /* } */

        bsg_cuda_print_stat_start(2);
        fft();
        bsg_cuda_print_stat_end(2);

        bsg_cuda_print_stat_start(3);
        for (int i = 0; i < NUM_POINTS; i++) {
            output[i] = fft_workset[i];
        }
        bsg_cuda_print_stat_end(3);

        bsg_cuda_print_stat_kernel_end();
        /* bsg_printf("\nResults:\n"); */
        /* for(int i = 0; i < NUM_POINTS; i++) { */
        /*     float rr = fft_workset[i].real(); */
        /*     float ii = fft_workset[i].imag(); */
        /*     bsg_printf("(%08X)+(%08X)i ", *(int*)&rr, *(int*)&ii); */
        /* } */
    }
    return 0;
}
