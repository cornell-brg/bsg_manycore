//====================================================================
// brg_8x8_cgra_xcel_pod.v
// Author : Peitian Pan
// Date   : Jan 10, 2021
//====================================================================
// A CGRA accelerator pod to be connected to an 8x16 HB manycore pod.
// This xcel pod instantiates two CGRA xcels.

module brg_8x8_cgra_xcel_pod
  import bsg_manycore_pkg::*;
  #( 
     x_cord_width_p               = "inv"
    ,y_cord_width_p               = "inv"
    ,data_width_p                 = "inv"
    ,addr_width_p                 = "inv"

    ,max_out_credits_p            = 32

    ,num_mesh_links               = 8
    ,num_mesh_links_per_cgra      = 4

    // Derived local parameters
    ,bsg_manycore_link_sif_width_lp = `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
    ,num_cgras                      = num_mesh_links / num_mesh_links_per_cgra
  )
  (   input clk_i
    , input reset_i

    // mesh network links
    , input  [num_mesh_links-1:0][bsg_manycore_link_sif_width_lp-1:0] link_sif_i
    , output [num_mesh_links-1:0][bsg_manycore_link_sif_width_lp-1:0] link_sif_o

    // Coordinate of the top-left tile (assuming the xcel pod is on the east
    // side of a manycore pod)
    , input   [x_cord_width_p-1:0]                my_x_i
    , input   [y_cord_width_p-1:0]                my_y_i
    );

    //--------------------------------------------------------------
    // Interface CGRA accelerators to the mesh network links
    //--------------------------------------------------------------

    for (genvar i = 0; i < num_cgras; i++) begin

      brg_8x8_cgra_xcel
      #(
         .x_cord_width_p               ( x_cord_width_p               )
        ,.y_cord_width_p               ( y_cord_width_p               )
        ,.data_width_p                 ( data_width_p                 )
        ,.addr_width_p                 ( addr_width_p                 )
        ,.max_out_credits_p            ( max_out_credits_p            )
        ,.num_mesh_links_per_cgra      ( num_mesh_links_per_cgra      )
      ) cgra_xcel
      (
         .clk_i      ( clk_i      )
        ,.reset_i    ( reset_i    )
        ,.link_sif_i ( link_sif_i[(i+1)*num_mesh_links_per_cgra-1:i*num_mesh_links_per_cgra] )
        ,.link_sif_o ( link_sif_o[(i+1)*num_mesh_links_per_cgra-1:i*num_mesh_links_per_cgra] )
        ,.my_x_i     ( my_x_i     )
        ,.my_y_i     ( y_cord_width_p'(my_y_i+i*num_mesh_links_per_cgra) )
      );

    end

endmodule
