// This kernel executes a series of CGRA instructions
// on an 8x8 CGRA accelerator

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_mutex.h"

#define XCEL_X_CORD 16
#define XCEL_Y_CORD 2

#define CONFIG_INST 0
#define LAUNCH_INST 1

#define CGRA_REG_CALC_GO          0
#define CGRA_REG_CFG_GO           1
#define CGRA_REG_ME_CFG_BASE_ADDR 2
#define CGRA_REG_PE_CFG_BASE_ADDR 3
#define CGRA_REG_PE_CFG_STRIDE    4
#define CGRA_REG_CFG_CMD          5
#define CGRA_REG_CFG_DONE         6
#define CGRA_REG_CALC_DONE        7

#define CGRA_BASE_SCRATCHPAD_WORD_ADDR 64

#define CGRA_REG_WR(ADDR, DATA)    \
  do {                                  \
    bsg_global_store(XCEL_X_CORD, XCEL_Y_CORD, ADDR << 2, DATA); \
  } while(0)

#define CGRA_REG_RD(ADDR, DATA)    \
  do {                                  \
    bsg_global_load(XCEL_X_CORD, XCEL_Y_CORD, ADDR << 2, DATA); \
  } while(0)

void stage_data(int* src, int size) {
  int i;
  bsg_remote_int_ptr xcel_base_ptr = bsg_global_ptr(XCEL_X_CORD, XCEL_Y_CORD, 0);

  // The first 64 words are mapped to registers
  size = size-CGRA_BASE_SCRATCHPAD_WORD_ADDR;

  src += CGRA_BASE_SCRATCHPAD_WORD_ADDR;
  xcel_base_ptr += CGRA_BASE_SCRATCHPAD_WORD_ADDR;

  for (i = 0; i < size; i++)
    *(xcel_base_ptr + i) = src[i];
}

void execute_config(int ME_addr, int PE_addr, int PE_stride, int Cmd) {
  // 2
  CGRA_REG_WR(CGRA_REG_ME_CFG_BASE_ADDR, ME_addr);
  // 3
  CGRA_REG_WR(CGRA_REG_PE_CFG_BASE_ADDR, PE_addr);
  // 4
  CGRA_REG_WR(CGRA_REG_PE_CFG_STRIDE,    PE_stride);
  // 5
  CGRA_REG_WR(CGRA_REG_CFG_CMD,          Cmd);
  // 1
  CGRA_REG_WR(CGRA_REG_CFG_GO,           1);
}

void execute_launch() {
  CGRA_REG_WR(CGRA_REG_CALC_GO, 1);
}

void read_back(int* dst, int verif_base_addr, int size) {
  int i;
  bsg_remote_int_ptr xcel_base_ptr = bsg_global_ptr(XCEL_X_CORD, XCEL_Y_CORD, 0);

  xcel_base_ptr += verif_base_addr;

  for (i = 0; i < size; i++)
    dst[i] = *(xcel_base_ptr + i);
}

int __attribute__ ((noinline)) kernel_cgra_xcel_hb_tapeout(
    int* mem_image,
    int mem_image_size,
    int* instructions,
    int* config_arg0, int* config_arg1,
    int* config_arg2, int* config_arg3,
    int instruction_size,
    int* results,
    int verif_base_addr,
    int result_size
  ) {
  int i, done = 0, cfg_IC = 0;

  bsg_remote_int_ptr xcel_base_ptr;

  bsg_set_tile_x_y();

  xcel_base_ptr = bsg_global_ptr(XCEL_X_CORD, XCEL_Y_CORD, 0);

  if((__bsg_x == 0) && (__bsg_y == 0)) {
    bsg_printf("[INFO] cgra-xcel starts!\n");

    //==========================================================
    // Stage data into xcel scratchpad
    //==========================================================

    stage_data(mem_image, mem_image_size);

    bsg_printf("[INFO] Config and data have been staged into CGRA scratchpad!\n");

    //==========================================================
    // Execute CGRA instructions by performing remote stores
    //==========================================================

    for (i = 0; i < instruction_size; i++)
      if (instructions[i] == CONFIG_INST) {
        execute_config(
            config_arg0[cfg_IC], config_arg1[cfg_IC],
            config_arg2[cfg_IC], config_arg3[cfg_IC]);
        cfg_IC++;
      } else if (instructions[i] == LAUNCH_INST) {
        execute_launch();
      }

    bsg_printf("[INFO] All CGRA instructions executed! Polling till CGRA finishes...\n");

    //==========================================================
    // Polling the status of the CGRA until it's done
    //==========================================================
    // TODO: this will generate tons of network traffic! the
    // CGRA should be able to signal the processor when it's
    // done...

    while( done == 0 ) {
      CGRA_REG_RD(CGRA_REG_CALC_DONE, done);
    }

    bsg_printf("[INFO] CGRA has finished computation!\n");

    //==========================================================
    // Store results from CGRA into DRAM
    //==========================================================

    read_back(results, verif_base_addr, result_size);
  }
}
