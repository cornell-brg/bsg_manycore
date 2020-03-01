//This kernel adds 2 vectors by invoking the accelerator

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"

// TODO: is there a way to malloc in the scratchpad?
#define MAX_ARRAY_SIZE 64

#define XCEL_X_CORD 0
#define XCEL_Y_CORD (bsg_global_Y-1)

// Only one tile is used to drive the accelerator and no synchronization is needed
/* #define BSG_TILE_GROUP_X_DIM bsg_tiles_X */
/* #define BSG_TILE_GROUP_Y_DIM bsg_tiles_Y */
/* #include "bsg_tile_group_barrier.h" */
/* INIT_TILE_GROUP_BARRIER(r_barrier, c_barrier, 0, bsg_tiles_X-1, 0, bsg_tiles_Y-1); */

int scratchpad_A[MAX_ARRAY_SIZE];
int scratchpad_B[MAX_ARRAY_SIZE];

// Xcel will write this variable to indicate it has finished
int done;

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

bsg_remote_int_ptr xcel_CSR_base_ptr;
Norm_NPA_s addr_A, addr_B, addr_signal;

int __attribute__ ((noinline)) xcel_configure(
    int N, Norm_NPA_s *A, Norm_NPA_s *B, Norm_NPA_s *signal) {
  // Number of elements
  *(xcel_CSR_base_ptr + CSR_NELEM_IDX) = N;

  // A address
  *(xcel_CSR_base_ptr + CSR_A_ADDR_HI_IDX) = A -> HI;
  *(xcel_CSR_base_ptr + CSR_A_ADDR_LO_IDX) = A -> LO;

  // B address
  *(xcel_CSR_base_ptr + CSR_B_ADDR_HI_IDX) = B -> HI;
  *(xcel_CSR_base_ptr + CSR_B_ADDR_LO_IDX) = B -> LO;

  // Signaling address
  *(xcel_CSR_base_ptr + CSR_SIG_ADDR_HI_IDX) = signal -> HI;
  *(xcel_CSR_base_ptr + CSR_SIG_ADDR_LO_IDX) = signal -> LO;

  // Xcel local memory destination
  *(xcel_CSR_base_ptr + CSR_DST_ADDR_IDX) = (int) &done;
}

int __attribute__ ((noinline)) kernel_vec_add_xcel(int *A, int *B, int *C, int N) {
  int i;
  volatile int *xcel_res_ptr;

  bsg_set_tile_x_y();

  xcel_CSR_base_ptr = bsg_global_ptr(XCEL_X_CORD, XCEL_Y_CORD, 0);

  if((__bsg_x == 0) && (__bsg_y == 0) && (N <= MAX_ARRAY_SIZE)) {
    /* bsg_printf("[INFO] vvadd-xcel starts!\n"); */
    // Copy data from DRAM into scratchpad
    for(i = 0; i < N; i++) {
      scratchpad_A[i] = A[i];
      scratchpad_B[i] = B[i];
    }

    // Setup the configs
    addr_A = (Norm_NPA_s) {
      .epa32 = (unsigned int) &scratchpad_A,
      .x8    = __bsg_grp_org_x,
      .y8    = __bsg_grp_org_y
    };
    addr_B = (Norm_NPA_s) {
      .epa32 = (unsigned int) &scratchpad_B,
      .x8    = __bsg_grp_org_x,
      .y8    = __bsg_grp_org_y
    };
    addr_signal = (Norm_NPA_s) {
      .epa32 = (unsigned int) &done,
      .x8    = __bsg_x + __bsg_grp_org_x,
      .y8    = __bsg_y + __bsg_grp_org_y
    };

    // Configure the xcel CSRs
    xcel_configure(N, &addr_A, &addr_B, &addr_signal);

    // Xcel go
    done = 0;
    *(xcel_CSR_base_ptr + CSR_CMD_IDX) = 1;

    // Wait till xcel is done
    bsg_wait_local_int(&done, 1);

    xcel_res_ptr = (volatile int*) bsg_global_ptr(XCEL_X_CORD, XCEL_Y_CORD, &done);

    // Copy result from scratchpad into DRAM
    for(i = 0; i < N; i++)
      C[i] = *(xcel_res_ptr + i);
  }
}
