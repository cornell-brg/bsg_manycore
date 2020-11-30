//This kernel adds 2 vectors by invoking the accelerator

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"

// TODO: is there a way to malloc in the scratchpad?
#define MAX_ARRAY_SIZE 8*16

/* #define NUM_VEC_ADD_XCEL bsg_global_X */
#define NUM_VEC_ADD_XCEL 1

#define XCEL_Y_CORD (bsg_global_Y)

int scratchpad_A[MAX_ARRAY_SIZE];
int scratchpad_B[MAX_ARRAY_SIZE];

// Xcel will write this variable to indicate it has finished
int done[NUM_VEC_ADD_XCEL];

enum {
  CSR_CMD_IDX = 0,
  CSR_DRAM_ENABLE_IDX,
  CSR_NELEM_IDX,
  CSR_A_ADDR_HI_IDX,
  CSR_A_ADDR_LO_IDX,
  CSR_B_ADDR_HI_IDX,
  CSR_B_ADDR_LO_IDX,
  CSR_SIG_ADDR_HI_IDX,
  CSR_SIG_ADDR_LO_IDX,
  CSR_DST_ADDR_IDX,
} CSR_IDX_e;

typedef union{
  struct {
    unsigned int  epa32;     // LSB
    unsigned char x8;
    unsigned char y8;
    unsigned char chip8;
    unsigned char reserved8; // MSB
  };
  struct {
    unsigned int LO;
    unsigned int HI;
  };
} Norm_NPA_s;

Norm_NPA_s addr_A, addr_B, addr_signal;

void __attribute__ ((noinline)) xcel_launch( int i ) {
  bsg_remote_int_ptr xcel_CSR_base_ptr = bsg_global_ptr(i, XCEL_Y_CORD, 0);
  *(xcel_CSR_base_ptr + CSR_CMD_IDX) = 1;
}

void __attribute__ ((noinline)) xcel_configure(
    int i, int N,
    int* remote_addr_A, int* remote_addr_B,
    int* local_addr_C, int* remote_addr_signal ) {

  bsg_remote_int_ptr xcel_CSR_base_ptr = bsg_global_ptr(i, XCEL_Y_CORD, 0);
  Norm_NPA_s addr_A, addr_B, addr_signal;

  // Setup the configs

  addr_A = (Norm_NPA_s) {
    .epa32 = (unsigned int) remote_addr_A,
    .x8    = __bsg_grp_org_x,
    .y8    = __bsg_grp_org_y
  };
  addr_B = (Norm_NPA_s) {
    .epa32 = (unsigned int) remote_addr_B,
    .x8    = __bsg_grp_org_x,
    .y8    = __bsg_grp_org_y
  };
  addr_signal = (Norm_NPA_s) {
    .epa32 = (unsigned int) remote_addr_signal,
    .x8    = __bsg_grp_org_x,
    .y8    = __bsg_grp_org_y
  };

  // Number of elements
  *(xcel_CSR_base_ptr + CSR_NELEM_IDX) = N;

  // A address
  *(xcel_CSR_base_ptr + CSR_A_ADDR_HI_IDX) = addr_A.HI;
  *(xcel_CSR_base_ptr + CSR_A_ADDR_LO_IDX) = addr_A.LO;

  // B address
  *(xcel_CSR_base_ptr + CSR_B_ADDR_HI_IDX) = addr_B.HI;
  *(xcel_CSR_base_ptr + CSR_B_ADDR_LO_IDX) = addr_B.LO;

  // Signaling address
  *(xcel_CSR_base_ptr + CSR_SIG_ADDR_HI_IDX) = addr_signal.HI;
  *(xcel_CSR_base_ptr + CSR_SIG_ADDR_LO_IDX) = addr_signal.LO;

  // Xcel local memory destination
  *(xcel_CSR_base_ptr + CSR_DST_ADDR_IDX) = (unsigned int) local_addr_C;
}

int __attribute__ ((noinline)) kernel_vec_add_xcel(int *A, int *B, int *C, int N, int ELEM_PER_XCEL) {
  int i, j;

  /* const int NUM_VEC_ADD_XCEL = N / ELEM_PER_XCEL; */

  int* vec_xcel_dmem_addr = (int*) ( (1<<12) );

  bsg_set_tile_x_y();

  if((__bsg_x == 0) && (__bsg_y == 0) && (N <= MAX_ARRAY_SIZE)) {

    /* bsg_printf("[INFO] vvadd-xcel starts!\n"); */

    // Copy data from DRAM into scratchpad of vcore 1,1

    for(i = 0; i < N; i++) {
      scratchpad_A[i] = A[i];
      scratchpad_B[i] = B[i];
    }

    // Clear done signals

    for(i = 0; i < NUM_VEC_ADD_XCEL; i++)
      done[i] = 0;

    // Configure all accelerators

    for(i = 0; i < NUM_VEC_ADD_XCEL; i++) {
      int input_vec_offset = i * ELEM_PER_XCEL;
      xcel_configure( i,
                      ELEM_PER_XCEL,
                      scratchpad_A + input_vec_offset,
                      scratchpad_B + input_vec_offset,
                      vec_xcel_dmem_addr,
                      done + i );
    }

    bsg_fence();

    // Launch all accelerators

    for(i = 0; i < NUM_VEC_ADD_XCEL; i++)
      xcel_launch( i );

    // Poll local scratchpad signals until all xcels are done

    for(i = 0; i < NUM_VEC_ADD_XCEL; i++)
      bsg_wait_local_int(done+i, 1);

    // Copy results from xcel scratchpads to DRAM

    for(i = 0; i < NUM_VEC_ADD_XCEL; i++)
      for(j = 0; j < ELEM_PER_XCEL; j++) {
        int val = -1;
        bsg_global_load(i, XCEL_Y_CORD, vec_xcel_dmem_addr+j, val);
        C[j] = val;
      }

    bsg_fence();
  }
}