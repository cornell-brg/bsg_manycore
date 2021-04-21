//====================================================================
// brg_cgra_pod_sync.v
// Author : Peitian Pan
// Date   : Apr 20, 2021
//====================================================================
// We need this wrapper to faciliate post-synth FF and post-apr SDF
// simulation. The TB generation utility should target the interface
// of this module.

module brg_cgra_pod_sync
  import bsg_manycore_pkg::*;
  import bsg_noc_pkg::*; // {P=0, W, E, N, S}
  import bsg_tag_pkg::*;
  #(  parameter addr_width_p="inv"
    , parameter data_width_p="inv"
    , parameter x_cord_width_p="inv"
    , parameter y_cord_width_p="inv"
    , parameter max_out_credits_p="inv"

    , parameter pod_y_cord="inv"

    , parameter ruche_factor_X_p="inv"
    , parameter num_pods_x_p="inv"
    , parameter num_tiles_x_p = "inv"
    , parameter num_tiles_y_p = "inv"
    , parameter pod_x_cord_width_p = "inv"
    , parameter pod_y_cord_width_p = "inv"

    , parameter link_sif_width_lp =
      `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
    , parameter ruche_x_link_sif_width_lp =
      `bsg_manycore_ruche_x_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)

    , localparam fwd_width_lp =
      `bsg_manycore_packet_width(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p)
    , localparam rev_width_lp =
      `bsg_manycore_return_packet_width(x_cord_width_p, y_cord_width_p, data_width_p)

    , localparam sdr_lg_fifo_depth_gp = 3
    , localparam sdr_lg_credit_to_token_decimation_gp = 0

    , localparam x_subcord_width_lp=`BSG_SAFE_CLOG2(num_tiles_x_p)
    , localparam y_subcord_width_lp=`BSG_SAFE_CLOG2(num_tiles_y_p)
  )
  (
    input clk_i
    , input cgra_xcel_clk_i
    , input reset_i

    , input async_uplink_reset_i
    , input async_downlink_reset_i
    , input async_downstream_reset_i
    , input async_token_reset_i

    , input  [3:0][link_sif_width_lp-1:0] hor_link_sif_i
    , output [3:0][link_sif_width_lp-1:0] hor_link_sif_o

    , input  [3:0][ruche_x_link_sif_width_lp-1:0] ruche_link_sif_i
    , output [3:0][ruche_x_link_sif_width_lp-1:0] ruche_link_sif_o
  );

  `declare_bsg_manycore_link_sif_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p);

  // Connections to CGRA
  logic [3:0]                   c_hor_io_fwd_link_clk_li;
  logic [3:0][fwd_width_lp-1:0] c_hor_io_fwd_link_data_li;
  logic [3:0]                   c_hor_io_fwd_link_v_li;
  logic [3:0]                   c_hor_io_fwd_link_token_lo;

  logic [3:0]                   c_hor_io_fwd_link_clk_lo;
  logic [3:0][fwd_width_lp-1:0] c_hor_io_fwd_link_data_lo;
  logic [3:0]                   c_hor_io_fwd_link_v_lo;
  logic [3:0]                   c_hor_io_fwd_link_token_li;

  logic [3:0]                   c_hor_io_rev_link_clk_li;
  logic [3:0][rev_width_lp-1:0] c_hor_io_rev_link_data_li;
  logic [3:0]                   c_hor_io_rev_link_v_li;
  logic [3:0]                   c_hor_io_rev_link_token_lo;

  logic [3:0]                   c_hor_io_rev_link_clk_lo;
  logic [3:0][rev_width_lp-1:0] c_hor_io_rev_link_data_lo;
  logic [3:0]                   c_hor_io_rev_link_v_lo;
  logic [3:0]                   c_hor_io_rev_link_token_li;

  //-------------------------------------------------------------
  // East side SDR interface
  //-------------------------------------------------------------
  // Reference implementation:
  // bigblade_pow_row/v/bsg_manycore_pod_row_sdr.v

  logic [3:0] sdr_e_core_reset_li, sdr_e_core_reset_lo;
  logic [3:0][x_cord_width_p-1:0] sdr_e_core_global_x_li, sdr_e_core_global_x_lo;
  logic [3:0][y_cord_width_p-1:0] sdr_e_core_global_y_li, sdr_e_core_global_y_lo;

  bsg_manycore_link_sif_s [3:0][S:N] sdr_e_ver_link_sif_li, sdr_e_ver_link_sif_lo;
  logic [3:0] sdr_e_async_uplink_reset_li,     sdr_e_async_uplink_reset_lo;
  logic [3:0] sdr_e_async_downlink_reset_li,   sdr_e_async_downlink_reset_lo;
  logic [3:0] sdr_e_async_downstream_reset_li, sdr_e_async_downstream_reset_lo;
  logic [3:0] sdr_e_async_token_reset_li,      sdr_e_async_token_reset_lo;

  // PP: only connect 4 links to the CGRA half pod
  for (genvar y = 0; y < 4; y++) begin: sdr_e_y
    bsg_manycore_link_ruche_to_sdr_east #(
      .lg_fifo_depth_p                  (sdr_lg_fifo_depth_gp)
      ,.lg_credit_to_token_decimation_p (sdr_lg_credit_to_token_decimation_gp)

      ,.x_cord_width_p      (x_cord_width_p)
      ,.y_cord_width_p      (y_cord_width_p)
      ,.addr_width_p        (addr_width_p)
      ,.data_width_p        (data_width_p)
      ,.ruche_factor_X_p    (ruche_factor_X_p)
    ) sdr_e (
      .core_clk_i       (clk_i)
      ,.core_reset_i    (sdr_e_core_reset_li[y])
      ,.core_reset_o    (sdr_e_core_reset_lo[y])
  
      ,.core_ver_link_sif_i   (sdr_e_ver_link_sif_li[y])
      ,.core_ver_link_sif_o   (sdr_e_ver_link_sif_lo[y])

      ,.core_hor_link_sif_i   (hor_link_sif_i[y])
      ,.core_hor_link_sif_o   (hor_link_sif_o[y])

      ,.core_ruche_link_i     (ruche_link_sif_i[y])
      ,.core_ruche_link_o     (ruche_link_sif_o[y])

      ,.core_global_x_i       (sdr_e_core_global_x_li[y])
      ,.core_global_y_i       (sdr_e_core_global_y_li[y])
      ,.core_global_x_o       (sdr_e_core_global_x_lo[y])
      ,.core_global_y_o       (sdr_e_core_global_y_lo[y])

      ,.async_uplink_reset_i      (sdr_e_async_uplink_reset_li[y])
      ,.async_downlink_reset_i    (sdr_e_async_downlink_reset_li[y])
      ,.async_downstream_reset_i  (sdr_e_async_downstream_reset_li[y])
      ,.async_token_reset_i       (sdr_e_async_token_reset_li[y])

      ,.async_uplink_reset_o      (sdr_e_async_uplink_reset_lo[y])
      ,.async_downlink_reset_o    (sdr_e_async_downlink_reset_lo[y])
      ,.async_downstream_reset_o  (sdr_e_async_downstream_reset_lo[y])
      ,.async_token_reset_o       (sdr_e_async_token_reset_lo[y])

      ,.io_fwd_link_clk_o       (c_hor_io_fwd_link_clk_li[y])
      ,.io_fwd_link_data_o      (c_hor_io_fwd_link_data_li[y])
      ,.io_fwd_link_v_o         (c_hor_io_fwd_link_v_li[y])
      ,.io_fwd_link_token_i     (c_hor_io_fwd_link_token_lo[y])

      ,.io_fwd_link_clk_i       (c_hor_io_fwd_link_clk_lo[y])
      ,.io_fwd_link_data_i      (c_hor_io_fwd_link_data_lo[y])
      ,.io_fwd_link_v_i         (c_hor_io_fwd_link_v_lo[y])
      ,.io_fwd_link_token_o     (c_hor_io_fwd_link_token_li[y])

      ,.io_rev_link_clk_o       (c_hor_io_rev_link_clk_li[y])
      ,.io_rev_link_data_o      (c_hor_io_rev_link_data_li[y])
      ,.io_rev_link_v_o         (c_hor_io_rev_link_v_li[y])
      ,.io_rev_link_token_i     (c_hor_io_rev_link_token_lo[y])

      ,.io_rev_link_clk_i       (c_hor_io_rev_link_clk_lo[y])
      ,.io_rev_link_data_i      (c_hor_io_rev_link_data_lo[y])
      ,.io_rev_link_v_i         (c_hor_io_rev_link_v_lo[y])
      ,.io_rev_link_token_o     (c_hor_io_rev_link_token_li[y])
    );

    // connect between sdr east
    if (y < 3) begin
      // core reset
      assign sdr_e_core_reset_li[y+1] = sdr_e_core_reset_lo[y];
      // core global cord
      assign sdr_e_core_global_x_li[y+1] = sdr_e_core_global_x_lo[y];
      assign sdr_e_core_global_y_li[y+1] = sdr_e_core_global_y_lo[y];
      // ver link
      assign sdr_e_ver_link_sif_li[y][S] = sdr_e_ver_link_sif_lo[y+1][N];
      assign sdr_e_ver_link_sif_li[y+1][N] = sdr_e_ver_link_sif_lo[y][S];
      // async reset
      assign sdr_e_async_uplink_reset_li[y+1] = sdr_e_async_uplink_reset_lo[y];
      assign sdr_e_async_downlink_reset_li[y+1] = sdr_e_async_downlink_reset_lo[y];
      assign sdr_e_async_downstream_reset_li[y+1] = sdr_e_async_downstream_reset_lo[y];
      assign sdr_e_async_token_reset_li[y+1] = sdr_e_async_token_reset_lo[y];
    end else begin
      // core reset
      assign sdr_e_core_reset_li[0] = reset_i;
      // core global cord
      assign sdr_e_core_global_x_li[0] = { (pod_x_cord_width_p)'(1+num_pods_x_p), (x_subcord_width_lp)'(0) };
      assign sdr_e_core_global_y_li[0] = { (pod_y_cord_width_p)'(1+2*pod_y_cord), (y_subcord_width_lp)'(0) };
      // ver link -- tieoff
      bsg_manycore_link_sif_tieoff #(
        .addr_width_p(addr_width_p)
        ,.data_width_p(data_width_p)
        ,.x_cord_width_p(x_cord_width_p)
        ,.y_cord_width_p(y_cord_width_p)
      ) sdr_ver_n0o_tieoff (
        .clk_i(clk_i)
        ,.reset_i(reset_i)
        ,.link_sif_i(sdr_e_ver_link_sif_lo[0][N])
        ,.link_sif_o(sdr_e_ver_link_sif_li[3][S])
      );
      bsg_manycore_link_sif_tieoff #(
        .addr_width_p(addr_width_p)
        ,.data_width_p(data_width_p)
        ,.x_cord_width_p(x_cord_width_p)
        ,.y_cord_width_p(y_cord_width_p)
      ) sdr_ver_n0i_tieoff (
        .clk_i(clk_i)
        ,.reset_i(reset_i)
        ,.link_sif_i(sdr_e_ver_link_sif_lo[3][S])
        ,.link_sif_o(sdr_e_ver_link_sif_li[0][N])
      );
      // async reset
      assign sdr_e_async_uplink_reset_li[0] = async_uplink_reset_i;
      assign sdr_e_async_downlink_reset_li[0] = async_downlink_reset_i;
      assign sdr_e_async_downstream_reset_li[0] = async_downstream_reset_i;
      assign sdr_e_async_token_reset_li[0] = async_token_reset_i;
    end

  end  // for: sdr_e_y

  //-------------------------------------------------------------
  // CGRA half pod instantiation
  //-------------------------------------------------------------

  logic [3:0][y_cord_width_p-1:0] c_global_y_cord_li;
  assign c_global_y_cord_li[0] = { (pod_y_cord_width_p)'(1+pod_y_cord), (y_subcord_width_lp)'(0) };
  assign c_global_y_cord_li[1] = { (pod_y_cord_width_p)'(1+pod_y_cord), (y_subcord_width_lp)'(1) };
  assign c_global_y_cord_li[2] = { (pod_y_cord_width_p)'(1+pod_y_cord), (y_subcord_width_lp)'(2) };
  assign c_global_y_cord_li[3] = { (pod_y_cord_width_p)'(1+pod_y_cord), (y_subcord_width_lp)'(3) };

  brg_cgra_pod #(
    .addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.max_out_credits_p(max_out_credits_p)
  ) cgra_bay (
    .clk_i(cgra_xcel_clk_i)
    ,.reset_i(reset_i)
    ,.global_y_cord_i(c_global_y_cord_li)

    ,.async_uplink_reset_i(async_uplink_reset_i)
    ,.async_downlink_reset_i(async_downlink_reset_i)
    ,.async_downstream_reset_i(async_downstream_reset_i)
    ,.async_token_reset_i(async_token_reset_i)

    ,.async_uplink_reset_o()
    ,.async_downlink_reset_o()
    ,.async_downstream_reset_o()
    ,.async_token_reset_o()

    ,.io_fwd_link_clk_o(c_hor_io_fwd_link_clk_lo)
    ,.io_fwd_link_data_o(c_hor_io_fwd_link_data_lo)
    ,.io_fwd_link_v_o(c_hor_io_fwd_link_v_lo)
    ,.io_fwd_link_token_i(c_hor_io_fwd_link_token_li)

    ,.io_fwd_link_clk_i(c_hor_io_fwd_link_clk_li)
    ,.io_fwd_link_data_i(c_hor_io_fwd_link_data_li)
    ,.io_fwd_link_v_i(c_hor_io_fwd_link_v_li)
    ,.io_fwd_link_token_o(c_hor_io_fwd_link_token_lo)

    ,.io_rev_link_clk_o(c_hor_io_rev_link_clk_lo)
    ,.io_rev_link_data_o(c_hor_io_rev_link_data_lo)
    ,.io_rev_link_v_o(c_hor_io_rev_link_v_lo)
    ,.io_rev_link_token_i(c_hor_io_rev_link_token_li)

    ,.io_rev_link_clk_i(c_hor_io_rev_link_clk_li)
    ,.io_rev_link_data_i(c_hor_io_rev_link_data_li)
    ,.io_rev_link_v_i(c_hor_io_rev_link_v_li)
    ,.io_rev_link_token_o(c_hor_io_rev_link_token_lo)
  );

endmodule
