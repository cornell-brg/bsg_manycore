/**
 *    brg_vvadd_xcel_network_rx.v
 *
 *    The slave interface of the VVADD accelerator. Inspired by the vanilla
 *    processor slave interface network_rx.
 *
 *    Author : Peitian Pan
 *    Date   : Feb 20, 2020
 */

module brg_vvadd_xcel_network_rx
  import bsg_manycore_pkg::*;
  import brg_vvadd_xcel_pkg::*;
  #(
      parameter data_width_p="inv"
    , parameter addr_width_p="inv"
    , parameter x_cord_width_p="inv"
    , parameter y_cord_width_p="inv"
    , parameter dmem_size_p="inv"
  )
  (
      input clk_i
    , input reset_i

    , input [x_cord_width_p-1:0] my_x_i
    , input [y_cord_width_p-1:0] my_y_i

    // network side
    , input v_i
    , input [data_width_p-1:0] data_i
    , input [(data_width_p>>3)-1:0] mask_i
    , input [addr_width_p-1:0] addr_i
    , input we_i
    , input bsg_manycore_load_info_s load_info_i
    , input [x_cord_width_p-1:0] src_x_cord_i
    , input [y_cord_width_p-1:0] src_y_cord_i
    , output logic yumi_o

    , output logic [data_width_p-1:0] returning_data_o
    , output logic returning_v_o

    // xcel side
    , output logic rx_is_CSR_o
    , output logic rx_is_local_mem_o
    , output logic rx_we_o
    , output [addr_width_p-1:0] rx_addr_o
    , output [data_width_p-1:0] rx_wdata_o
    , output [(data_width_p>>3)-1:0] rx_mask_o
    , output logic rx_v_o
    , input  logic rx_yumi_i

    , input  [data_width_p-1:0] rx_returning_data_i
    , input  logic rx_returning_v_i
  );

  //------------------------------------------------------------------
  // address decoding
  //------------------------------------------------------------------
  // The way RX checks if a transaction is processor data access is
  // to see if the address is in [dmem_size, 2*dmem_size). Through
  // some simple experiments, it seems like all data structures 
  // declared in the device code lives in range [dmem_size, 2*dmem_size)
  // instead of [0, dmem_size). Maybe this is because the icache has
  // the same size as dmem_size and it occupies the lower range?

  logic [addr_width_p-1:0] xcel_local_addr;
  logic is_tile_csr_addr;
  logic is_dram_enable_addr;
  logic is_xcel_CSR_addr;
  
  // this is how vanilla core detects CSR addresses

  assign is_tile_csr_addr = addr_i[epa_word_addr_width_gp-1]
    & (addr_i[addr_width_p-1:epa_word_addr_width_gp] == '0);
  assign is_dram_enable_addr = is_tile_csr_addr
    & (addr_i[epa_word_addr_width_gp-2:0] == 'd4);
  assign is_xcel_CSR_addr = (~is_tile_csr_addr) & (addr_i < CSR_NUM_lp);

  assign xcel_local_addr = is_dram_enable_addr ? (addr_width_p)'(CSR_DRAM_ENABLE_IDX)
                                               : addr_i;

  //------------------------------------------------------------------
  // connect network side to xcel side
  //------------------------------------------------------------------

  assign rx_we_o = we_i;
  assign rx_addr_o = xcel_local_addr;
  assign rx_wdata_o = data_i;
  assign rx_mask_o = mask_i;
  assign rx_v_o = v_i;
  assign yumi_o = rx_yumi_i;

  assign returning_data_o = rx_returning_data_i;
  assign returning_v_o    = rx_returning_v_i;

  assign rx_is_CSR_o = is_dram_enable_addr | is_xcel_CSR_addr;
  assign rx_is_local_mem_o = (addr_i >= dmem_size_p) & (addr_i < 2*dmem_size_p);

  // synopsys translate_off
  always_ff @ (negedge clk_i) begin
    if (v_i & ~(is_dram_enable_addr | is_xcel_CSR_addr | rx_is_local_mem_o)) begin
      $error("[ERROR][VVADD-XCEL-RX] Unrecognized request to addr %h of value %h", addr_i, data_i);
      $finish();
    end

    if (v_i & we_i) begin
      if (is_dram_enable_addr) begin
        if (data_i[0])
          $display("[INFO][VVADD-XCEL-RX] Enabling DRAM ctrl at (%d, %d)", my_y_i, my_x_i);
        else
          $display("[INFO][VVADD-XCEL-RX] Disabling DRAM ctrl at (%d, %d)", my_y_i, my_x_i);
      end else begin
        $display("[INFO][VVADD-XCEL-RX] Writing vvadd-xcel addr %h with %h at (%d, %d)", rx_addr_o, rx_wdata_o, my_y_i, my_x_i);
      end
    end

    if (v_i & ~we_i) begin
      $display("[INFO][VVADD-XCEL-RX] Reading vvadd-xcel addr %h at (%d, %d)", rx_addr_o, my_y_i, my_x_i);
    end

    if (v_i & is_xcel_CSR_addr) begin
      if (we_i)
        $display("[INFO][VVADD-XCEL-RX] Writing vvadd-xcel CSR of index %h with %h at (%d, %d)", rx_addr_o, rx_wdata_o, my_y_i, my_x_i);
      else
        $display("[INFO][VVADD-XCEL-RX] Reading vvadd-xcel CSR of index %h at (%d, %d)", rx_addr_o, my_y_i, my_x_i);
    end

    if (returning_v_o) begin
      $display("[INFO][VVADD-XCEL-RX] Responding prev request with data %h at (%d, %d)", returning_data_o, my_y_i, my_x_i);
    end
  end
  // synopsys translate_on

endmodule
