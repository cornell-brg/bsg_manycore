// This kernel adds 2 vectors by invoking the accelerators

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"

/* #define ELEMS_PER_XCEL 4 */
#define ELEMS_PER_XCEL 16

/* #define NUM_VEC_ADD_XCEL 16 */
#define NUM_VEC_ADD_XCEL 1

// TODO: is there a way to malloc in the scratchpad?
#define MAX_ARRAY_SIZE (ELEMS_PER_XCEL*NUM_VEC_ADD_XCEL)

#define XCEL_X_CORD_FIRST_COL 0
#define XCEL_X_CORD_LAST_COL  (bsg_global_X-1)

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

int __attribute__ ((noinline)) xcel_load_result( int i, int* local_addr ) {
  int y = i % (bsg_global_Y-1);
  int x = i >= (bsg_global_Y-1) ? XCEL_X_CORD_LAST_COL : XCEL_X_CORD_FIRST_COL;
  int val = -1;
  bsg_printf("[kernel] loading %p @ (%d, %d)\n", local_addr, y, x);
  /* bsg_global_load(x, y, local_addr, val); */
  bsg_global_load(0, 2, local_addr, val);
  return val;
}

void __attribute__ ((noinline)) xcel_launch( int i ) {
  int y = i % (bsg_global_Y-1);
  int x = i >= (bsg_global_Y-1) ? XCEL_X_CORD_LAST_COL : XCEL_X_CORD_FIRST_COL;
  bsg_printf("[kernel] launching (%d, %d)\n", y, x);
  /* bsg_remote_int_ptr xcel_CSR_base_ptr = bsg_global_ptr(x, y, 0); */
  bsg_remote_int_ptr xcel_CSR_base_ptr = bsg_global_ptr(0, 2, 0);
  *(xcel_CSR_base_ptr + CSR_CMD_IDX) = 1;
}

void __attribute__ ((noinline)) xcel_configure(
    int i, int N,
    int* remote_addr_A, int* remote_addr_B,
    int* local_addr_C, int* remote_addr_signal ) {

  int y = i % (bsg_global_Y-1);
  int x = i >= (bsg_global_Y-1) ? XCEL_X_CORD_LAST_COL : XCEL_X_CORD_FIRST_COL;
  bsg_printf("[kernel] configuring (%d, %d)\n", y, x);
  /* bsg_remote_int_ptr xcel_CSR_base_ptr = bsg_global_ptr(x, y, 0); */
  bsg_remote_int_ptr xcel_CSR_base_ptr = bsg_global_ptr(0, 2, 0);
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

int __attribute__ ((noinline)) kernel_vec_add_toplevel_xcel(int *A, int *B, int *C, int N, int ELEM_PER_XCEL) {
  int i, j;

  /* const int NUM_VEC_ADD_XCEL = N / ELEM_PER_XCEL; */

  int* vec_xcel_dmem_addr = (int*) ( (1<<12) );

  bsg_set_tile_x_y();

  if((__bsg_x == 0) && (__bsg_y == 0) && (N <= MAX_ARRAY_SIZE)) {

    bsg_printf("[INFO] vvadd-xcel is alive!\n");
    bsg_printf("[INFO] Y = %d, X = %d\n", __bsg_grp_org_y, __bsg_grp_org_x);

    // Copy data from DRAM into scratchpad of vcore 1,1

    for(i = 0; i < N; i++) {
      scratchpad_A[i] = A[i];
      scratchpad_B[i] = B[i];
    }

    /* for(i = 0; i < N; i++) */
    /*   bsg_printf("[INFO] A[%d]=%d\n", i, scratchpad_A[i]); */

    /* for(i = 0; i < N; i++) */
    /*   bsg_printf("[INFO] B[%d]=%d\n", i, scratchpad_B[i]); */

    // Clear done signals

    for(i = 0; i < NUM_VEC_ADD_XCEL; i++)
      done[i] = 0;

    // Configure all accelerators

    for(i = 0; i < NUM_VEC_ADD_XCEL; i++) {
      /* int input_vec_offset = i * ELEM_PER_XCEL; */
      /* xcel_configure( i, */
      /*                 ELEM_PER_XCEL, */
      /*                 scratchpad_A + input_vec_offset, */
      /*                 scratchpad_B + input_vec_offset, */
      /*                 vec_xcel_dmem_addr, */
      /*                 done + i ); */
      int input_vec_offset = i * ELEMS_PER_XCEL;
      xcel_configure( i,
                      ELEMS_PER_XCEL,
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
      /* for(j = 0; j < ELEM_PER_XCEL; j++) { */
      /*   C[j] = xcel_load_result(i, vec_xcel_dmem_addr+j); */
      /* } */
      for(j = 0; j < ELEMS_PER_XCEL; j++) {
        C[j] = xcel_load_result(i, vec_xcel_dmem_addr+j);
      }

    bsg_fence();
  }
}
