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
  #(  parameter addr_width_p="inv"
    , parameter data_width_p="inv"
    , parameter x_cord_width_p="inv"
    , parameter y_cord_width_p="inv"
    , parameter ruche_factor_X_p="inv"
    , parameter num_row_p="inv"
    , parameter max_out_credits_p="inv"

    , parameter link_sif_width_lp =
      `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
    , parameter ruche_x_link_sif_width_lp =
      `bsg_manycore_ruche_x_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)

    , parameter num_mesh_links_per_cgra_lp = 4
  )
  (
      input xcel_clk_i
    , input xcel_reset_i

    , input mc_clk_i
    , input mc_reset_i

    // This pod only takes links from one side
    , input  [num_row_p-1:0][link_sif_width_lp-1:0] mc_hor_links_i
    , output [num_row_p-1:0][link_sif_width_lp-1:0] mc_hor_links_o

    , input  [num_row_p-1:0][ruche_x_link_sif_width_lp-1:0] mc_ruche_links_i
    , output [num_row_p-1:0][ruche_x_link_sif_width_lp-1:0] mc_ruche_links_o

    , input [x_cord_width_p-1:0] global_x_i
    , input [num_row_p-1:0][y_cord_width_p-1:0] global_y_i
  );

  `declare_bsg_manycore_link_sif_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p);

  //=========================================================================
  // bsg_manycore_hor_io_router_column
  //=========================================================================
  // Since we assume the CGRA xcel pod is on the east side, we explicitly tie
  // off the east side of links.

  logic [S:N][link_sif_width_lp-1:0] mc_ver_links_li;
  logic [S:N][link_sif_width_lp-1:0] mc_ver_links_lo;

  bsg_manycore_link_sif_s [num_row_p-1:0] mc_proc_links_li;
  bsg_manycore_link_sif_s [num_row_p-1:0] mc_proc_links_lo;

  logic [num_row_p-1:0][E:W][link_sif_width_lp-1:0] mc_hor_links_li;
  logic [num_row_p-1:0][E:W][link_sif_width_lp-1:0] mc_hor_links_lo;

  logic [num_row_p-1:0][E:W][ruche_x_link_sif_width_lp-1:0] mc_ruche_links_li;
  logic [num_row_p-1:0][E:W][ruche_x_link_sif_width_lp-1:0] mc_ruche_links_lo;

  bsg_manycore_hor_io_router_column #(
    .addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.ruche_factor_X_p(ruche_factor_X_p)

    ,.num_row_p(num_row_p)
    
    ,.tieoff_west_p({num_row_p{1'b0}})
    ,.tieoff_east_p({num_row_p{1'b1}})
  ) io_rtr_col (
    .clk_i(mc_clk_i)
    ,.reset_i(mc_reset_i)

    // Tie off unused vertical links
    ,.ver_link_sif_i(mc_ver_links_li)
    ,.ver_link_sif_o(mc_ver_links_lo)

    ,.proc_link_sif_i(mc_proc_links_li)
    ,.proc_link_sif_o(mc_proc_links_lo)

    ,.hor_link_sif_i(mc_hor_links_li)
    ,.hor_link_sif_o(mc_hor_links_lo)

    ,.ruche_link_i(mc_ruche_links_li)
    ,.ruche_link_o(mc_ruche_links_lo)
    
    ,.global_x_i(global_x_i)
    ,.global_y_i(global_y_i)
  );

  // Tie off vertical links on the north and south sides

  bsg_manycore_link_sif_tieoff #(
    .addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
  ) ver_n_tieoff (
    .clk_i(mc_clk_i)
    ,.reset_i(mc_reset_i)
    ,.link_sif_i(mc_ver_links_lo[N])
    ,.link_sif_o(mc_ver_links_li[N])
  );

  bsg_manycore_link_sif_tieoff #(
    .addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
  ) ver_s_tieoff (
    .clk_i(mc_clk_i)
    ,.reset_i(mc_reset_i)
    ,.link_sif_i(mc_ver_links_lo[S])
    ,.link_sif_o(mc_ver_links_li[S])
  );

  // Handle horizontal links

  for (genvar i = 0; i < num_row_p; i++) begin: hor_tieoff
    // Bring in the mesh network links from the west side
    assign mc_hor_links_o[i] = mc_hor_links_lo[i][W];
    assign mc_hor_links_li[i][W] = mc_hor_links_i[i];
    // hor link E gets '0
    assign mc_hor_links_li[i][E] = '0;

    // Bring in the ruche links from the west side
    bsg_ruche_buffer #(
      .width_p(ruche_x_link_sif_width_lp)
      ,.ruche_factor_p(ruche_factor_X_p)
      ,.ruche_stage_p(2)
      ,.harden_p(0)
    ) rb_col_to_mc (
      .i(mc_ruche_links_lo[i][W])
      ,.o(mc_ruche_links_o[i])
    );
    bsg_ruche_buffer #(
      .width_p(ruche_x_link_sif_width_lp)
      ,.ruche_factor_p(ruche_factor_X_p)
      ,.ruche_stage_p(0)
      ,.harden_p(0)
    ) rb_mc_to_col (
      .i(mc_ruche_links_i[i])
      ,.o(mc_ruche_links_li[i][W])
    );
  end

  //=========================================================================
  // CDC
  //=========================================================================

  // These links are li/lo to/from xcel tile. I moved them here to avoid the
  // forward reference warning.
  bsg_manycore_link_sif_s [num_row_p-1:0] xcel_proc_links_li;
  bsg_manycore_link_sif_s [num_row_p-1:0] xcel_proc_links_lo;

  for (genvar i = 0; i < num_row_p; i++) begin: rof2
    bsg_async_noc_link #(
      .width_p($bits(bsg_manycore_fwd_link_sif_s)-2),
      .lg_size_p(3)
    ) fwd_cdc (
      .aclk_i(xcel_clk_i)
      ,.areset_i(xcel_reset_i)
      ,.bclk_i(mc_clk_i)
      ,.breset_i(mc_reset_i)
      ,.alink_i(xcel_proc_links_lo[i].fwd)
      ,.alink_o(xcel_proc_links_li[i].fwd)
      ,.blink_i(mc_proc_links_lo[i].fwd)
      ,.blink_o(mc_proc_links_li[i].fwd)
    );
    bsg_async_noc_link #(
      .width_p($bits(bsg_manycore_rev_link_sif_s)-2),
      .lg_size_p(3)
    ) rev_cdc (
      .aclk_i(xcel_clk_i)
      ,.areset_i(xcel_reset_i)
      ,.bclk_i(mc_clk_i)
      ,.breset_i(mc_reset_i)
      ,.alink_i(xcel_proc_links_lo[i].rev)
      ,.alink_o(xcel_proc_links_li[i].rev)
      ,.blink_i(mc_proc_links_lo[i].rev)
      ,.blink_o(mc_proc_links_li[i].rev)
    );
  end

  //=========================================================================
  // brg_8x8_cgra_xcel_tile
  //=========================================================================
  // CGRA accelerator tile

  brg_8x8_cgra_xcel_tile #(
    .x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.data_width_p(data_width_p)
    ,.addr_width_p(addr_width_p)
    ,.max_out_credits_p(max_out_credits_p)
    ,.num_mesh_links(num_row_p)
    ,.num_mesh_links_per_cgra(num_mesh_links_per_cgra_lp)
  ) pod (
    .clk_i(xcel_clk_i)
    ,.reset_i(xcel_reset_i)
    ,.link_sif_i(xcel_proc_links_li)
    ,.link_sif_o(xcel_proc_links_lo)
    ,.my_x_i(global_x_i)
    ,.my_y_i(global_y_i[0])
  );

endmodule
