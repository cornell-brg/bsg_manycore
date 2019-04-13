
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

//dram_ch_addr_width_p set to DRAM_CH_ADDR_BITS-2
#define DRAM_CH_ADDR_BITS 18

enum {
  CSR_GO = 0, // go/fetch result
  CSR_FUNC,
  CSR_ADDR,
  CSR_NUM
} CSRs;

int main()
{
  bsg_set_tile_x_y();

  // Only core x,y=0,0 prepares data, since we set tile_group_Y = 1
  if ((__bsg_x == 0) && (__bsg_y == 0)) {
    bsg_remote_int_ptr dram = bsg_dram_ptr(0x0);
    dram[0] = 0x6666;
    dram[1] = 0x66666;
    dram[2] = 0x666666;
    dram[3] = 0x6666666;

    bsg_printf("----- Phase 1 ----- read existing value\n");
    for (int i=0; i<4; ++i) {
      bsg_remote_int_ptr xcel_csr_base_ptr = bsg_global_ptr( i, 2, 0 );
      xcel_csr_base_ptr[CSR_FUNC] = 0; // read
      xcel_csr_base_ptr[CSR_ADDR] = &(dram[i]);
      xcel_csr_base_ptr[CSR_GO  ] = 1;
      bsg_printf("xcel #%d returns read result = 0x%x\n",
                 i, xcel_csr_base_ptr[CSR_GO] );
    }

    bsg_printf("----- Phase 2 ----- xcel write value to dram\n");
    for (int i=0; i<4; ++i) {
      bsg_remote_int_ptr xcel_csr_base_ptr = bsg_global_ptr( i, 2, 0 );
      xcel_csr_base_ptr[CSR_FUNC] = 1; // write
      xcel_csr_base_ptr[CSR_ADDR] = &(dram[i]);
      xcel_csr_base_ptr[CSR_GO  ] = 1;
      bsg_printf("xcel #%d returns write result = 0x%x\n",
                  i, xcel_csr_base_ptr[CSR_GO] );
    }
    bsg_printf("----- Phase 3 ----- xcel read new value\n");
    for (int i=0; i<4; ++i) {
      bsg_remote_int_ptr xcel_csr_base_ptr = bsg_global_ptr( i, 2, 0 );
      xcel_csr_base_ptr[CSR_FUNC] = 0; // read
      xcel_csr_base_ptr[CSR_ADDR] = &(dram[i]);
      xcel_csr_base_ptr[CSR_GO  ] = 1;
      bsg_printf("xcel #%d returns read result = 0x%x\n",
                 i, xcel_csr_base_ptr[CSR_GO] );
    }
    bsg_finish();
  }

  bsg_wait_while(1);
}

