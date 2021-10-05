// This kernel performs radix of two DIT FFT

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#include <cmath>
#include <complex>

#define NUM_POINTS      256
#define LOG2_NUM_POINTS 8

/*******************************************************************************
 * Efficient sinf and cosf implementation
*******************************************************************************/
// Return sin(-2*pi*x/NUM_POINTS) and cos(-2*pi*x/NUM_POINTS)

// 65 elements = 260B
float sinf_pi_over_2[(NUM_POINTS>>2)+1] = {
    0.0000000000000000f,
    0.0245412285229123f,
    0.0490676743274180f,
    0.0735645635996674f,
    0.0980171403295606f,
    0.1224106751992162f,
    0.1467304744553617f,
    0.1709618887603012f,
    0.1950903220161282f,
    0.2191012401568698f,
    0.2429801799032639f,
    0.2667127574748984f,
    0.2902846772544623f,
    0.3136817403988915f,
    0.3368898533922201f,
    0.3598950365349881f,
    0.3826834323650898f,
    0.4052413140049899f,
    0.4275550934302821f,
    0.4496113296546065f,
    0.4713967368259976f,
    0.4928981922297840f,
    0.5141027441932217f,
    0.5349976198870972f,
    0.5555702330196022f,
    0.5758081914178453f,
    0.5956993044924334f,
    0.6152315905806268f,
    0.6343932841636455f,
    0.6531728429537768f,
    0.6715589548470183f,
    0.6895405447370668f,
    0.7071067811865475f,
    0.7242470829514669f,
    0.7409511253549591f,
    0.7572088465064845f,
    0.7730104533627370f,
    0.7883464276266062f,
    0.8032075314806448f,
    0.8175848131515837f,
    0.8314696123025452f,
    0.8448535652497070f,
    0.8577286100002721f,
    0.8700869911087113f,
    0.8819212643483549f,
    0.8932243011955153f,
    0.9039892931234433f,
    0.9142097557035307f,
    0.9238795325112867f,
    0.9329927988347388f,
    0.9415440651830208f,
    0.9495281805930367f,
    0.9569403357322089f,
    0.9637760657954398f,
    0.9700312531945440f,
    0.9757021300385286f,
    0.9807852804032304f,
    0.9852776423889412f,
    0.9891765099647810f,
    0.9924795345987100f,
    0.9951847266721968f,
    0.9972904566786902f,
    0.9987954562051724f,
    0.9996988186962042f,
    1.0000000000000000f
};

inline float
opt_fft_sinf(int x) {
    const int No2 = NUM_POINTS >> 1;
    const int No4 = NUM_POINTS >> 2;
    if ((x >= No4) && (x < No2)) {
        x = No2 - x;
    }
    else if (x >= No2) {
        bsg_fail();
    }
    // consult lookup table for sinf(x)
    return -sinf_pi_over_2[x];
}

inline float
opt_fft_cosf(int x) {
    const int No2 = NUM_POINTS >> 1;
    const int No4 = NUM_POINTS >> 2;
    if ((x >= No4) && (x < No2)) {
        return -sinf_pi_over_2[x-No4];
    }
    else if (x >= No2) {
        bsg_fail();
        return 0.0;
    } else {
        return sinf_pi_over_2[No4-x];
    }
}

/* End of sinf and cosf implementation */

typedef std::complex<float> FP32Complex;

FP32Complex *input;
FP32Complex *output;
FP32Complex fft_workset[NUM_POINTS];

/* void */
/* debug_print_complex(const FP32Complex *list, const int N, const char *msg) { */
/*     bsg_printf("%s\n", msg); */
/*     for(int i = 0; i < N; i++) { */
/*         float rr = list[i].real(); */
/*         float ii = list[i].imag(); */
/*         bsg_printf("(%08X)+(%08X)i ", *(int*)&rr, *(int*)&ii); */
/*     } */
/* } */

inline void
opt_data_transfer(FP32Complex *dst, const FP32Complex *src, const int N) {
    int i = 0;
    for (; i < N-3; i += 4) {
        register FP32Complex tmp0 = src[i    ];
        register FP32Complex tmp1 = src[i + 1];
        register FP32Complex tmp2 = src[i + 2];
        register FP32Complex tmp3 = src[i + 3];
        asm volatile("": : :"memory");
        dst[i    ] = tmp0;
        dst[i + 1] = tmp1;
        dst[i + 2] = tmp2;
        dst[i + 3] = tmp3;

    }
    // fixup
    for (; i < N; i++)
        dst[i] = src[i];
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
    int even_idx, odd_idx, n = 2, lshift = LOG2_NUM_POINTS-1;
    FP32Complex exp_val, tw_val, even_val, odd_val;

    while (n <= NUM_POINTS) {
        for (int i = 0; i < NUM_POINTS; i += n) {
            for (int k = 0; k < n/2; k++) {
                even_idx = i+k;
                odd_idx  = even_idx + n/2;

                exp_val = FP32Complex(opt_fft_cosf(k << lshift),
                                      opt_fft_sinf(k << lshift));

                even_val = fft_workset[even_idx];
                odd_val  = fft_workset[odd_idx];

                tw_val = exp_val*odd_val;

                fft_workset[even_idx] = even_val + tw_val;
                fft_workset[odd_idx]  = even_val - tw_val;
            }
        }
        n = n * 2;
        lshift--;
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
        bsg_cuda_print_stat_start(1);
        opt_data_transfer(fft_workset, input, NUM_POINTS);
        bsg_cuda_print_stat_end(1);

        bsg_cuda_print_stat_start(2);
        opt_bit_reverse(fft_workset, NUM_POINTS);
        bsg_cuda_print_stat_end(2);
        /* debug_print_complex(fft_workset, NUM_POINTS, "After bit reverse"); */

        bsg_cuda_print_stat_start(3);
        fft();
        bsg_cuda_print_stat_end(3);

        bsg_cuda_print_stat_start(4);
        opt_data_transfer(output, fft_workset, NUM_POINTS);
        bsg_cuda_print_stat_end(4);

        bsg_cuda_print_stat_kernel_end();
        /* debug_print_complex(fft_workset, NUM_POINTS, "Results"); */
    }
    return 0;
}
