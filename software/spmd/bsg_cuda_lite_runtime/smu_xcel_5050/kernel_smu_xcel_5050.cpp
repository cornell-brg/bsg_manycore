// This kernel streams out a padded matrix using an SMU

#ifndef SMU_XCEL_5050
#define SMU_XCEL_5050
#endif

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_tile_group_barrier.hpp"

#define NROWS 16
#define NCOLS 16

#define GO            (0  << 2)
#define PADDING       (2  << 2)
#define SRC_BASE_ADDR (3  << 2)
#define SRC_X_STRIDE  (4  << 2)
#define SRC_X_COUNT   (5  << 2)
#define SRC_Y_STRIDE  (6  << 2)
#define SRC_Y_COUNT   (7  << 2)
#define DST_BASE_ADDR (8  << 2)
#define DST_ACK_ADDR  (9  << 2)
#define DST_CORD      (10 << 2)

#define SMU_REG_WR(X, Y, ADDR, DATA)    \
  do {                                  \
    bsg_global_store(X, Y, ADDR, DATA); \
  } while(0)

// Allocate space in DRAM
int arr[NROWS*NCOLS] __attribute__ ((section (".dram"))) = {0};

// Allocate space in scratchpad
int buf[NROWS*NCOLS];

// ACK variable
int ack;

// Tile group barrier
bsg_barrier<16,8> g_barrier;

void __attribute__ ((noinline)) smu_test(
    int  test_number,
    int  padding,       int* array,
    int  src_x_stride,  int  src_x_count,
    int  src_y_stride,  int  src_y_count,
    int* dst_base_addr, int* dst_ack_addr
)
{
  bsg_printf("[INFO] (%d, %d) SMU Test#%d starts!\n", bsg_y, bsg_x, test_number);
  bsg_printf("[INFO] (%d, %d) dst_base_addr=%u, dst_ack_addr=%u\n", bsg_y, bsg_x, dst_base_addr, dst_ack_addr);

  int dst_cord = ((__bsg_y+2) << 16) | (__bsg_x+1);

  // Clear local ACK variable
  *dst_ack_addr = 0;

  int SMU_X_CORD = __bsg_x;
  int SMU_Y_CORD = __bsg_y+2;

  // Configure SMU
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, PADDING,       padding       );
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, SRC_BASE_ADDR, array         );
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, SRC_X_STRIDE,  src_x_stride  );
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, SRC_X_COUNT,   src_x_count   );
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, SRC_Y_STRIDE,  src_y_stride  );
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, SRC_Y_COUNT,   src_y_count   );
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, DST_BASE_ADDR, dst_base_addr );
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, DST_ACK_ADDR,  dst_ack_addr  );
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, DST_CORD,      dst_cord      );

  bsg_printf("[INFO]   (%d, %d) Launching SMU...\n", bsg_y, bsg_x);

  // Launch SMU
  SMU_REG_WR( SMU_X_CORD, SMU_Y_CORD, GO, 1 );

  bsg_printf("[INFO]   (%d, %d) Waiting for SMU...\n", bsg_y, bsg_x);

  // Polling local ACK variable
  bsg_wait_while(*dst_ack_addr == 0);

  bsg_printf("[INFO]   (%d, %d) Verifying results...\n", bsg_y, bsg_x);

  // Verify results
  for(int y = 0; y < src_y_count; y++)
    for(int x = 0; x < src_x_count; x++) {
      int buf_idx = y*src_x_count+x;
      int result  = 0xdead0000+y*NCOLS+x;
      if( (x == 0)             && (padding & 0x1) ) result = 0; // pad W
      if( (x == src_x_count-1) && (padding & 0x2) ) result = 0; // pad E
      if( (y == 0)             && (padding & 0x4) ) result = 0; // pad N
      if( (y == src_y_count-1) && (padding & 0x8) ) result = 0; // pad S
      if( dst_base_addr[buf_idx] != result ) {
        bsg_printf("[FAIL] (%d, %d) SMU Test#%d failed: expects 0x%x but got 0x%x\n",
                   bsg_y, bsg_x, test_number, result, dst_base_addr[buf_idx] );
        bsg_fail();
      }
    }

  bsg_printf("[INFO]   (%d, %d) Test passed!\n", bsg_y, bsg_x);
}

extern "C" __attribute__ ((noinline))
int kernel_smu_xcel_5050() {
  bsg_set_tile_x_y();

  // Note that the origin is tile (2,1)...
  if((__bsg_x == 0) && (__bsg_y == 0)) {

    // Start by initializing arr and ref

    for(int i = 0; i < NROWS; i++)
      for(int j = 0; j < NCOLS; j++)
        arr[i*NCOLS+j] = 0xdead0000+i*NCOLS+j;
  }

  g_barrier.sync();

  if ( __bsg_x == 0 ) {

    // Test cases

    // No padding, load 8x8
    smu_test( 1, 0x0, &arr[0], 1<<2, 8,  (NCOLS)<<2, 8,  &buf[0], &ack);
    // Pad W and S, load 16x8
    smu_test( 2, 0x9, &arr[0], 1<<2, 16, (NCOLS)<<2, 8,  &buf[0], &ack);
    // Pad four sides, load 16x16
    smu_test( 3, 0xF, &arr[0], 1<<2, 16, (NCOLS)<<2, 16, &buf[0], &ack);

  }

  g_barrier.sync();
  return 0;
}
