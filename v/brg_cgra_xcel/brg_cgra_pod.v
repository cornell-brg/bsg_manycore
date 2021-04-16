//====================================================================
// brg_cgra_pod.v
// Author : Peitian Pan
// Date   : Jan 15, 2021
//====================================================================
// CGRA accelerator pod that connects to east side of the ruche-enabled
// pod. This modules instantiates hor_io_router_column to directly
// interface to the mesh network links and the ruche links.

module brg_cgra_pod
  import bsg_manycore_pkg::*;
  import bsg_noc_pkg::*; // {P=0, W, E, N, S}
  import bsg_tag_pkg::*;
  #(  parameter addr_width_p="inv"
    , parameter data_width_p="inv"
    , parameter x_cord_width_p="inv"
    , parameter y_cord_width_p="inv"
    , parameter max_out_credits_p="inv"

    , localparam fwd_width_lp =
      `bsg_manycore_packet_width(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p)
    , localparam rev_width_lp =
      `bsg_manycore_return_packet_width(x_cord_width_p, y_cord_width_p, data_width_p)

    , localparam sdr_lg_fifo_depth_gp = 3
    , localparam sdr_lg_credit_to_token_decimation_gp = 0
  )
  (
    input clk_i
    , input reset_i

    , input [3:0][y_cord_width_p-1:0] global_y_cord_i

    , input        async_uplink_reset_i
    , input        async_downlink_reset_i
    , input        async_downstream_reset_i
    , input        async_token_reset_i
 
    , output logic async_uplink_reset_o
    , output logic async_downlink_reset_o
    , output logic async_downstream_reset_o
    , output logic async_token_reset_o
 
    , output logic [3:0]                   io_fwd_link_clk_o
    , output logic [3:0][fwd_width_lp-1:0] io_fwd_link_data_o
    , output logic [3:0]                   io_fwd_link_v_o
    , input [3:0]                          io_fwd_link_token_i
 
    , input [3:0]                          io_fwd_link_clk_i
    , input [3:0][fwd_width_lp-1:0]        io_fwd_link_data_i
    , input [3:0]                          io_fwd_link_v_i
    , output logic [3:0]                   io_fwd_link_token_o
 
    , output logic [3:0]                   io_rev_link_clk_o
    , output logic [3:0][rev_width_lp-1:0] io_rev_link_data_o
    , output logic [3:0]                   io_rev_link_v_o
    , input [3:0]                          io_rev_link_token_i
 
    , input [3:0]                          io_rev_link_clk_i
    , input [3:0][rev_width_lp-1:0]        io_rev_link_data_i
    , input [3:0]                          io_rev_link_v_i
    , output logic [3:0]                   io_rev_link_token_o
  );

  `declare_bsg_manycore_link_sif_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p);

  bsg_manycore_link_sif_s [3:0] proc_link_sif_li, proc_link_sif_lo;

  //=========================================================================
  // Reset
  //=========================================================================

  // Latch reset to meet timing
  logic reset_r;
  bsg_dff #(
    .width_p(1)
  ) reset_reg (
    .clk_i(clk_i)
    ,.data_i(reset_i)
    ,.data_o(reset_r)
  );

  logic uplink_reset_sync;
  bsg_sync_sync #(
    .width_p(1)
  ) up_bss (
    .oclk_i(clk_i)
    ,.iclk_data_i(async_uplink_reset_i)
    ,.oclk_data_o(uplink_reset_sync)
  );

  logic downstream_reset_sync;
  bsg_sync_sync #(
    .width_p(1)
  ) down_bss (
    .oclk_i(clk_i)
    ,.iclk_data_i(async_downstream_reset_i)
    ,.oclk_data_o(downstream_reset_sync)
  );

  assign async_uplink_reset_o     = async_uplink_reset_i;
  assign async_downlink_reset_o   = async_downlink_reset_i;
  assign async_downstream_reset_o = async_downstream_reset_i;
  assign async_token_reset_o      = async_token_reset_i;

  //=========================================================================
  // SDR
  //=========================================================================

  for (genvar i = 0; i < 4; i++) begin : links
    bsg_link_sdr #(
      .width_p(fwd_width_lp)
      ,.lg_fifo_depth_p(sdr_lg_fifo_depth_gp)
      ,.lg_credit_to_token_decimation_p(sdr_lg_credit_to_token_decimation_gp)
    ) fwd_sdr (
      .core_clk_i(clk_i)
      ,.core_uplink_reset_i(uplink_reset_sync)
      ,.core_downstream_reset_i(downstream_reset_sync)
      ,.async_downlink_reset_i(async_downlink_reset_i)
      ,.async_token_reset_i(async_token_reset_i)

      ,.core_data_i(proc_link_sif_lo[i].fwd.data)
      ,.core_v_i(proc_link_sif_lo[i].fwd.v)
      ,.core_ready_o(proc_link_sif_li[i].fwd.ready_and_rev)

      ,.core_data_o(proc_link_sif_li[i].fwd.data)
      ,.core_v_o(proc_link_sif_li[i].fwd.v)
      ,.core_yumi_i(proc_link_sif_li[i].fwd.v & proc_link_sif_lo[i].fwd.ready_and_rev)

      ,.link_clk_o(io_fwd_link_clk_o[i])
      ,.link_data_o(io_fwd_link_data_o[i])
      ,.link_v_o(io_fwd_link_v_o[i])
      ,.link_token_i(io_fwd_link_token_i[i])

      ,.link_clk_i(io_fwd_link_clk_i[i])
      ,.link_data_i(io_fwd_link_data_i[i])
      ,.link_v_i(io_fwd_link_v_i[i])
      ,.link_token_o(io_fwd_link_token_o[i])
    );

    bsg_link_sdr #(
      .width_p(rev_width_lp)
      ,.lg_fifo_depth_p(sdr_lg_fifo_depth_gp)
      ,.lg_credit_to_token_decimation_p(sdr_lg_credit_to_token_decimation_gp)
    ) rev_sdr (
      .core_clk_i(clk_i)
      ,.core_uplink_reset_i(uplink_reset_sync)
      ,.core_downstream_reset_i(downstream_reset_sync)
      ,.async_downlink_reset_i(async_downlink_reset_i)
      ,.async_token_reset_i(async_token_reset_i)

      ,.core_data_i(proc_link_sif_lo[i].rev.data)
      ,.core_v_i(proc_link_sif_lo[i].rev.v)
      ,.core_ready_o(proc_link_sif_li[i].rev.ready_and_rev)

      ,.core_data_o(proc_link_sif_li[i].rev.data)
      ,.core_v_o(proc_link_sif_li[i].rev.v)
      ,.core_yumi_i(proc_link_sif_li[i].rev.v & proc_link_sif_lo[i].rev.ready_and_rev)

      ,.link_clk_o(io_rev_link_clk_o[i])
      ,.link_data_o(io_rev_link_data_o[i])
      ,.link_v_o(io_rev_link_v_o[i])
      ,.link_token_i(io_rev_link_token_i[i])

      ,.link_clk_i(io_rev_link_clk_i[i])
      ,.link_data_i(io_rev_link_data_i[i])
      ,.link_v_i(io_rev_link_v_i[i])
      ,.link_token_o(io_rev_link_token_o[i])
    );
  end

  //=========================================================================
  // brg_8x8_cgra_xcel
  //=========================================================================

  // NOTE: the x cord is hard coded!
  wire [x_cord_width_p-1:0] global_x_cord_li = 'h20;

  brg_8x8_cgra_xcel#(
    .x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.data_width_p(data_width_p)
    ,.addr_width_p(addr_width_p)
    ,.max_out_credits_p(max_out_credits_p)
  ) pod (
    .clk_i(clk_i)
    ,.reset_i(reset_r)
    ,.link_sif_i(proc_link_sif_li)
    ,.link_sif_o(proc_link_sif_lo)
    ,.my_x_i(global_x_cord_li)
    // TODO: use multiple endpoints
    ,.my_y_i(global_y_cord_i[0])
  );

endmodule
