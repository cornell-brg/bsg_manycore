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

/* void */
/* debug_print_complex(const FP32Complex *list, const int N, const char *msg) { */
/*     bsg_printf("%s\n", msg); */
/*     for(int i = 0; i < N; i++) { */
/*         float rr = list[i].real(); */
/*         float ii = list[i].imag(); */
/*         bsg_printf("(%08X)+(%08X)i ", *(int*)&rr, *(int*)&ii); */
/*     } */
/* } */

/* inline void */
/* opt_data_transfer(FP32Complex *dst, const FP32Complex *src, const int N) { */
/*     int i = 0; */
/*     for (; i < N-4; i += 4) { */
/*         register FP32Complex tmp0 = src[i    ]; */
/*         register FP32Complex tmp1 = src[i + 1]; */
/*         register FP32Complex tmp2 = src[i + 2]; */
/*         register FP32Complex tmp3 = src[i + 3]; */
/*         asm volatile("": : :"memory"); */
/*         dst[i    ] = tmp0; */
/*         dst[i + 1] = tmp1; */
/*         dst[i + 2] = tmp2; */
/*         dst[i + 3] = tmp3; */

/*     } */
/*     // fixup */
/*     for (; i < N; i++) */
/*         dst[i] = src[i]; */
/* } */

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

inline void
opt_bit_reverse(FP32Complex *list, const int N) {
    // Efficient bit reverse
    // http://wwwa.pikara.ne.jp/okojisan/otfft-en/cooley-tukey.html
    // The idea is to perform a reversed binary +1. The inner for loop
    // executes M times, where M is the number of carries in the
    // reversed binary +1 operation.
    for (int i = 0, j = 1; j < N-1; j++) {
        for (int k = N >> 1; k > (i ^= k); k >>= 1);
        // after the for loop above i is bit-reversed j
        if (i < j) {
            register FP32Complex tmp = list[i];
            list[i] = list[j];
            list[j] = tmp;
        }
    }
}

void
fft() {
    int even_idx, odd_idx, n = 2;
    float k_div_n;
    FP32Complex exp_val, tw_val, even_val, odd_val;
    const float PI = 3.14159265358979323846;
    /* FP32Complex ImagURoot = FP32Complex(0.0, 1.0); */
    FP32Complex exp_val_common = FP32Complex(0.0, -2.0f*PI);

    while (n <= NUM_POINTS) {
        for (int i = 0; i < NUM_POINTS; i += n) {
            for (int k = 0; k < n/2; k++) {
                even_idx = i+k;
                odd_idx  = even_idx + n/2;

                k_div_n = (float) k / (float) n;
                exp_val = exp_val_common*k_div_n;

                even_val = fft_workset[even_idx];
                odd_val  = fft_workset[odd_idx];

                tw_val = std::exp(exp_val)*odd_val;

                fft_workset[even_idx] = even_val + tw_val;
                fft_workset[odd_idx]  = even_val - tw_val;
            }
        }
        n = n * 2;
    }

    /* opt_data_transfer(output, fft_workset, NUM_POINTS); */
    for (int i = 0; i < NUM_POINTS; i++) {
        output[i] = fft_workset[i];
    }
}

extern "C" __attribute__ ((noinline))
int
kernel_opt_single_fft(FP32Complex *in, FP32Complex *out, int N) {
    if (bsg_id == 0) {
        bsg_cuda_print_stat_kernel_start();
        input = in;
        output = out;

        /* debug_print_complex(input, NUM_POINTS, "Before bit reverse"); */
        /* opt_data_transfer(fft_workset, input, NUM_POINTS); */
        /* opt_bit_reverse(fft_workset, NUM_POINTS); */
        bsg_cuda_print_stat_start(1);
        fft_swizzle(0, 1);
        bsg_cuda_print_stat_end(1);
        /* debug_print_complex(fft_workset, NUM_POINTS, "After bit reverse"); */

        bsg_cuda_print_stat_start(2);
        fft();
        bsg_cuda_print_stat_end(2);

        bsg_cuda_print_stat_kernel_end();
        /* debug_print_complex(fft_workset, NUM_POINTS, "Results"); */
    }
    return 0;
}
