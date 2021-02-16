/**
 *  main.c
 *  
 *  cgra_xcel_hb_tapeout
 *
 *  Run a series of CGRA regression tests based on the auto-generated
 *  test vectors.
 *
 */

#include "cgra_xcel_hb_tapeout.h"

// mem_image: address of bitstream in DRAM
// mem_image_size: size of the bitstream (number of words)
// instructions: address of instructions in DRAM
// config_arg0: address of arg0 in DRAM
// config_arg1: address of arg1 in DRAM
// config_arg2: address of arg2 in DRAM
// config_arg3: address of arg3 in DRAM
// instruction_size: size of instructions (number of words)
// verif_base_addr, result_size: copy result_size words starting from
//                               verif_base_addr (word address) in the
//                               CGRA EVA space
// reference: address of the reference array in DRAM
void run_test( int test_num, const char* test_name,
               const int* mem_image,
               int mem_image_size,
               const int* instructions,
               const int* config_arg0, const int* config_arg1,
               const int* config_arg2, const int* config_arg3,
               int instruction_size,
               int verif_base_addr,
               int result_size,
               const int* reference ) {

    int i, done = 0, cfg_IC = 0;
    bsg_printf("[INFO] Test Case #%d: %s\n", test_num, test_name);

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
    // Compare results with reference
    //==========================================================

    check_against_ref(reference, verif_base_addr, result_size);
}

int main()
{

  int i;
  bsg_set_tile_x_y();

  if ((__bsg_x == 0) && (__bsg_y == 0)) {

    bsg_printf("[INFO] cgra-xcel starts!\n");

    if (is_run_all_tests()) {
      for (i = 0; i < NUM_TEST_VECTORS; i++) {
        run_test( i, test_name[i],
                  bstrm_addr[i], bstrm_size[i],
                  insts_addr[i], arg0_addr[i], arg1_addr[i],
                  arg2_addr[i], arg3_addr[i], inst_size[i],
                  verif_base[i], result_size[i], ref_addr[i]);
      }
    } else {
      i = get_test_index();
      run_test( i, test_name[i],
                bstrm_addr[i], bstrm_size[i],
                insts_addr[i], arg0_addr[i], arg1_addr[i],
                arg2_addr[i], arg3_addr[i], inst_size[i],
                verif_base[i], result_size[i], ref_addr[i]);
    }

    bsg_finish();

  }

  bsg_wait_while(1);

}
