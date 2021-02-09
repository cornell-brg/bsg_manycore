//====================================================================
// brg_8x8_cgra_xcel_pod_w_hor_io_rtr_col.v
// Author : Peitian Pan
// Date   : Jan 15, 2021
//====================================================================
// CGRA accelerator pod that connects to east side of the ruche-enabled
// pod. This modules instantiates hor_io_router_column to directly
// interface to the mesh network links and the ruche links.

module brg_8x8_cgra_xcel_pod_w_hor_io_rtr_col
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
      input clk_i
    , input reset_i

    // This pod only takes links from one side
    , input  [num_row_p-1:0][link_sif_width_lp-1:0] hor_link_sif_i
    , output [num_row_p-1:0][link_sif_width_lp-1:0] hor_link_sif_o

    , input  [num_row_p-1:0][ruche_x_link_sif_width_lp-1:0] ruche_link_i
    , output [num_row_p-1:0][ruche_x_link_sif_width_lp-1:0] ruche_link_o

    , input [x_cord_width_p-1:0] global_x_i
    , input [num_row_p-1:0][y_cord_width_p-1:0] global_y_i
  );

  //=========================================================================
  // bsg_manycore_hor_io_router_column
  //=========================================================================
  // Since we assume the CGRA xcel pod is on the east side, we explicitly tie
  // off the east side of links.

  logic [S:N][link_sif_width_lp-1:0] ver_link_sif_li;
  logic [S:N][link_sif_width_lp-1:0] ver_link_sif_lo;

  logic [num_row_p-1:0][link_sif_width_lp-1:0] proc_link_sif_li;
  logic [num_row_p-1:0][link_sif_width_lp-1:0] proc_link_sif_lo;

  logic [num_row_p-1:0][E:W][link_sif_width_lp-1:0] hor_link_sif_li;
  logic [num_row_p-1:0][E:W][link_sif_width_lp-1:0] hor_link_sif_lo;

  logic [num_row_p-1:0][E:W][ruche_x_link_sif_width_lp-1:0] ruche_link_li;
  logic [num_row_p-1:0][E:W][ruche_x_link_sif_width_lp-1:0] ruche_link_lo;

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
    .clk_i(clk_i)
    ,.reset_i(reset_i)

    // Tie off unused vertical links
    ,.ver_link_sif_i(ver_link_sif_li)
    ,.ver_link_sif_o(ver_link_sif_lo)

    ,.proc_link_sif_i(proc_link_sif_li)
    ,.proc_link_sif_o(proc_link_sif_lo)

    ,.hor_link_sif_i(hor_link_sif_li)
    ,.hor_link_sif_o(hor_link_sif_lo)

    ,.ruche_link_i(ruche_link_li)
    ,.ruche_link_o(ruche_link_lo)
    
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
    .clk_i(clk_i)
    ,.reset_i(reset_i)
    ,.link_sif_i(ver_link_sif_lo[N])
    ,.link_sif_o(ver_link_sif_li[N])
  );

  bsg_manycore_link_sif_tieoff #(
    .addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
  ) ver_s_tieoff (
    .clk_i(clk_i)
    ,.reset_i(reset_i)
    ,.link_sif_i(ver_link_sif_lo[S])
    ,.link_sif_o(ver_link_sif_li[S])
  );

  // Handle horizontal links

  for (genvar i = 0; i < num_row_p; i++) begin: hor_tieoff
    // Bring in the mesh network links from the west side
    assign hor_link_sif_o[i] = hor_link_sif_lo[i][W];
    assign hor_link_sif_li[i][W] = hor_link_sif_i[i];

    // Bring in the ruche links from the west side
    bsg_ruche_buffer #(
      .width_p(ruche_x_link_sif_width_lp)
      ,.ruche_factor_p(ruche_factor_X_p)
      ,.ruche_stage_p(2)
      ,.harden_p(0)
    ) rb_col_to_mc (
      .i(ruche_link_lo[i][W])
      ,.o(ruche_link_o[i])
    );
    bsg_ruche_buffer #(
      .width_p(ruche_x_link_sif_width_lp)
      ,.ruche_factor_p(ruche_factor_X_p)
      ,.ruche_stage_p(0)
      ,.harden_p(0)
    ) rb_mc_to_col (
      .i(ruche_link_i[i])
      ,.o(ruche_link_li[i][W])
    );
  end

  //=========================================================================
  // brg_8x8_cgra_xcel_pod
  //=========================================================================
  // CGRA accelerator pod

  logic [num_row_p-1:0][link_sif_width_lp-1:0] cgra_xcel_pod_link_sif_li;
  logic [num_row_p-1:0][link_sif_width_lp-1:0] cgra_xcel_pod_link_sif_lo;

  brg_8x8_cgra_xcel_pod #(
    .x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.data_width_p(data_width_p)
    ,.addr_width_p(addr_width_p)
    ,.max_out_credits_p(max_out_credits_p)
    ,.num_mesh_links(num_row_p)
    ,.num_mesh_links_per_cgra(num_mesh_links_per_cgra_lp)
  ) pod (
    .clk_i(clk_i)
    ,.reset_i(reset_i)
    ,.link_sif_i(cgra_xcel_pod_link_sif_li)
    ,.link_sif_o(cgra_xcel_pod_link_sif_lo)
    ,.my_x_i(global_x_i)
    ,.my_y_i(global_y_i[0])
  );

  for (genvar i = 0; i < num_row_p; i++) begin
    assign cgra_xcel_pod_link_sif_li[i] = proc_link_sif_lo[i];
    assign proc_link_sif_li[i] = cgra_xcel_pod_link_sif_lo[i];
  end

endmodule
