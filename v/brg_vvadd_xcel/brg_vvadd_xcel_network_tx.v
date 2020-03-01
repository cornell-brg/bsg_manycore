/**
 *    brg_vvadd_xcel_network_tx.v
 *
 *    The master interface of the VVADD accelerator. Inspired by the vanilla
 *    processor master interface network_tx.
 *
 *    Author : Peitian Pan
 *    Date   : Feb 20, 2020
 */


module brg_vvadd_xcel_network_tx
  import bsg_manycore_pkg::*;
  #(
      parameter data_width_p="inv"
    , parameter addr_width_p="inv"
    , parameter x_cord_width_p="inv"
    , parameter y_cord_width_p="inv"

    , parameter max_out_credits_p="inv"

    , localparam credit_counter_width_lp=$clog2(max_out_credits_p+1)

    , localparam packet_width_lp=
      `bsg_manycore_packet_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
  )
  (
      input clk_i
    , input reset_i

    , input logic [x_cord_width_p-1:0] my_x_i
    , input logic [y_cord_width_p-1:0] my_y_i
 
    // endpoint side
    , output logic v_o
    , output [packet_width_lp-1:0] packet_o
    , input  ready_i
    , input  [credit_counter_width_lp-1:0] credits_i

    , input  [data_width_p-1:0] returned_data_i
    , input  [4:0] returned_reg_id_i
    , input  returned_v_i
    , input  bsg_manycore_return_packet_type_e returned_pkt_type_i
    , output logic returned_yumi_o
    , input  returned_fifo_full_i

    // xcel side
    , input  tx_v_i
    , input  tx_fetching_i
    , input  [addr_width_p-1:0] tx_addr_i
    , input  [addr_width_p-1:0] tx_signal_addr_i
    , input  [4:0] tx_reg_id_i
    , output tx_ready_o
    , output [credit_counter_width_lp-1:0] tx_credits_o

    , output [data_width_p-1:0] tx_returned_data_o
    , output [4:0] tx_returned_reg_id_o
    , output tx_returned_v_o
  );

  `declare_bsg_manycore_packet_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p);
  bsg_manycore_packet_payload_u payload_empty;
  wire bsg_manycore_packet_s packet_s;

  assign payload_empty.load_info_s.reserved = 'b0;
  assign payload_empty.load_info_s.load_info.float_wb = 'b0;
  assign payload_empty.load_info_s.load_info.icache_fetch = 'b0;
  assign payload_empty.load_info_s.load_info.is_unsigned_op = 'b1;
  assign payload_empty.load_info_s.load_info.is_byte_op = 'b0;
  assign payload_empty.load_info_s.load_info.is_hex_op = 'b0;
  assign payload_empty.load_info_s.load_info.part_sel = 'b0;

  assign v_o = tx_v_i;
  assign tx_ready_o = ready_i;
  assign tx_credits_o = credits_i;

  // TODO: remove hard-coded tile cords?
  assign packet_s = '{
     addr       : tx_fetching_i ? tx_addr_i : tx_signal_addr_i
    ,op         : tx_fetching_i ? e_remote_load : e_remote_store
    ,op_ex      : {(data_width_p>>3){1'b1}}
    ,reg_id     : tx_reg_id_i
    ,payload    : tx_fetching_i ? payload_empty : data_width_p'(1)
    ,src_x_cord : my_x_i
    ,src_y_cord : my_y_i
    ,x_cord     : x_cord_width_p'(0)
    ,y_cord     : y_cord_width_p'(1)
  };
  assign packet_o = packet_s;

  assign tx_returned_data_o = returned_data_i;
  assign tx_returned_reg_id_o = returned_reg_id_i;
  assign tx_returned_v_o = returned_v_i;

  // The protocol requires the response interface of master should always be
  // ready after it initiates a request.
  assign returned_yumi_o = returned_v_i;

  // synopsys translate_off
  always_ff@(negedge clk_i) begin
    if( v_o & ready_i ) begin
      if( tx_fetching_i )
        $display("[INFO][VVADD-XCEL-TX] Load request sent to %h at (1, 0)", tx_addr_i);
      else
        $display("[INFO][VVADD-XCEL-TX] Signaling the processor that vvadd-xcel has finished...");
    end
    if( returned_v_i & returned_yumi_o ) begin
      $display("[INFO][VVADD-XCEL-TX] Received data = %h, reg_id = %h", returned_data_i, returned_reg_id_i);
    end
  end
  // synopsys translate_on

endmodule
