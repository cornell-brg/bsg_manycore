// Header file of cgra_xcel_hb_tapeout.c

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define str(s) #s
#define xstr(s) str(s)

#define TEST_NAME xstr(TEST_SEL)

// Useful macros

#define POD_CORD_X_WIDTH 3
#define POD_CORD_Y_WIDTH 4

#define TILE_CORD_X_WIDTH 4
#define TILE_CORD_Y_WIDTH 3

// Assume there is only one manycore pod for now
#define XCEL_X_CORD ((2) << TILE_CORD_X_WIDTH | (0))
#define XCEL_Y_CORD ((1) << TILE_CORD_Y_WIDTH | (0))

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

// Include the auto-generated test vectors
#include "cgra_test_vectors/cgra_test_vectors.dat"

void stage_data(const int* src, int size) {
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

void check_against_ref(const int* ref, int verif_base_addr, int size) {
  int i, val;
  bsg_remote_int_ptr xcel_base_ptr = bsg_global_ptr(XCEL_X_CORD, XCEL_Y_CORD, 0);

  xcel_base_ptr += verif_base_addr;

  for (i = 0; i < size; i++) {
    val = *(xcel_base_ptr+i);
    /* bsg_printf("[CHECK] ref[%d] = %d, result[%d] = %d\n", */
    /*            i, ref[i], i, val); */
    if (ref[i] != val) {
      bsg_printf("[FAILED] ref[%d] = %d but result[%d] = %d!\n",
                 i, ref[i], i, val);
      bsg_fail();
    }
  }
  bsg_printf("[passed]\n");
}

int get_str_len(const char* str) {
  int len = 0;
  for(len = 0; str[len] != '\0'; len++) ;
  return len;
}

int is_str_same(const char* str1, const char* str2) {
  int len1 = get_str_len(str1);
  int len2 = get_str_len(str2);
  if (len1 != len2)
    return 0;
  for (int i = 0; i < len1; i++)
    if (str1[i] != str2[i])
      return 0;
  return 1;
}

int is_run_all_tests() {
  const char* str_all = "all";
  const char* str_test = TEST_NAME;
  int str_len = get_str_len(str_test);

  if (str_len != 3)
    return 0;

  for (int i = 0; i < str_len; i++)
    if (str_all[i] != str_test[i])
      return 0;

  return 1;
}

int get_test_index() {
  const char* str_test = TEST_NAME;
  for (int i = 0; i < NUM_TEST_VECTORS; i++)
    if (is_str_same(test_name[i], str_test))
      return i;
  bsg_printf("[FAILED] The given test name %s is not registered!\n", str_test);
  bsg_fail();
}
