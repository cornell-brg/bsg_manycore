//====================================================================
// brg_8x8_cgra_xcel.v
// Author : Peitian Pan
// Date   : Jan 10, 2021
//====================================================================
// CGRA accelerator with HB endpoint interfaces.

module brg_8x8_cgra_xcel
  import bsg_manycore_pkg::*;
  #( 
     x_cord_width_p               = "inv"
    ,y_cord_width_p               = "inv"
    ,data_width_p                 = "inv"
    ,addr_width_p                 = "inv"
    ,max_out_credits_p            = 32

    // Derived local parameters
    ,packet_width_lp                = `bsg_manycore_packet_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
    ,bsg_manycore_link_sif_width_lp = `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
  )
  (   input clk_i
    , input reset_i

    // mesh network
    , input  [3:0][bsg_manycore_link_sif_width_lp-1:0] link_sif_i
    , output [3:0][bsg_manycore_link_sif_width_lp-1:0] link_sif_o

    // Coordinate of the top-left tile (assuming the xcel pod is on the east
    // side of a manycore pod)
    , input   [x_cord_width_p-1:0]                my_x_i
    , input   [y_cord_width_p-1:0]                my_y_i
  );

    //--------------------------------------------------------------
    // Endpoint standard
    //--------------------------------------------------------------

    `declare_bsg_manycore_packet_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p);

    // endpoint slave interface ports
    logic [3:0]                             in_v_lo         ;
    logic [3:0][data_width_p-1:0]           in_data_lo      ;
    logic [3:0][(data_width_p>>3)-1:0]      in_mask_lo      ;
    logic [3:0][addr_width_p-1:0]           in_addr_lo      ;
    logic [3:0]                             in_we_lo        ;
    bsg_manycore_load_info_s [3:0]          in_load_info_lo ;
    logic [3:0][x_cord_width_p-1:0]         in_src_x_cord_lo;
    logic [3:0][y_cord_width_p-1:0]         in_src_y_cord_lo;
    logic [3:0]                             in_yumi_li      ;

    logic [3:0][data_width_p-1:0]           returning_data_li;
    logic [3:0]                             returning_v_li   ;

    // endpoint master interface ports
    logic [3:0]                                  out_v_li              ;
    logic [3:0][packet_width_lp-1:0]             out_packet_li         ;
    logic [3:0]                                  out_credit_or_ready_lo;
    logic [3:0][$clog2(max_out_credits_p+1)-1:0] out_credits_lo        ;

    logic [3:0][data_width_p-1:0]           returned_data_lo     ;
    logic [3:0][4:0]                        returned_reg_id_lo   ;
    logic [3:0]                             returned_v_lo        ;
    bsg_manycore_return_packet_type_e [3:0] returned_pkt_type_lo ;
    logic [3:0]                             returned_yumi_li     ;
    logic [3:0]                             returned_fifo_full_lo;

    logic [3:0]                             unused_returned_credit_v_lo        ;
    logic [3:0][4:0]                        unused_returned_credit_reg_id_lo   ;

    for (genvar i = 0; i < 4; i++) begin : ep

      bsg_manycore_endpoint_standard
      #(
         .x_cord_width_p        ( x_cord_width_p    )
        ,.y_cord_width_p        ( y_cord_width_p    )
        ,.fifo_els_p            ( 4                 )
        ,.data_width_p          ( data_width_p      )
        ,.addr_width_p          ( addr_width_p      )
        ,.max_out_credits_p     ( max_out_credits_p )
      ) endpoint_gs
      (  .clk_i ( clk_i )
        ,.reset_i ( reset_i )

        // mesh network
        ,.link_sif_i( link_sif_i[i] )
        ,.link_sif_o( link_sif_o[i] )
        ,.global_x_i( my_x_i )
        ,.global_y_i( y_cord_width_p'(my_y_i+i) )

        // slave request
        ,.in_v_o                ( in_v_lo[i]              )
        ,.in_data_o             ( in_data_lo[i]           )
        ,.in_mask_o             ( in_mask_lo[i]           )
        ,.in_addr_o             ( in_addr_lo[i]           )
        ,.in_we_o               ( in_we_lo[i]             )
        ,.in_load_info_o        ( in_load_info_lo[i]      )
        ,.in_src_x_cord_o       ( in_src_x_cord_lo[i]     )
        ,.in_src_y_cord_o       ( in_src_y_cord_lo[i]     )
        ,.in_yumi_i             ( in_yumi_li[i]           )

        // slave response
        ,.returning_data_i      ( returning_data_li[i]    )
        ,.returning_v_i         ( returning_v_li[i]       )

        // local outgoing data interface (does not include credits)
        // Tied up all the outgoing signals
        // master request
        ,.out_v_i               ( out_v_li[i]               )
        ,.out_packet_i          ( out_packet_li[i]          )
        ,.out_credit_or_ready_o ( out_credit_or_ready_lo[i] )
        ,.out_credits_o         ( out_credits_lo[i]         )

        // local returned data interface
        // Like the memory interface, processor should always ready be to
        // handle the returned data
        // master reponse
        ,.returned_data_r_o     ( returned_data_lo[i]      )
        ,.returned_reg_id_r_o   ( returned_reg_id_lo[i]    )
        ,.returned_v_r_o        ( returned_v_lo[i]         )
        ,.returned_pkt_type_r_o ( returned_pkt_type_lo[i]  ) // new field, do not use it for now
        ,.returned_yumi_i       ( returned_yumi_li[i]      )
        ,.returned_fifo_full_o  ( returned_fifo_full_lo[i] )

        ,.returned_credit_v_r_o ( unused_returned_credit_v_lo[i] )
        ,.returned_credit_reg_id_r_o ( unused_returned_credit_reg_id_lo[i] )
      );

    end

    //--------------------------------------------------------------
    // Interface CGRA accelerator to the endpoints
    //--------------------------------------------------------------

    HBEndpointCGRAXcel_8x8Array_4x4KBSpads hb_cgra_xcel (
      .clk                   ( clk_i                  ),
      .reset                 ( reset_i                ),

      .my_x_i                ( my_x_i                 ),
      .my_y_i                ( my_y_i                 ),

      .in_addr_i             ( in_addr_lo[0]             ),
      .in_data_i             ( in_data_lo[0]             ),
      .in_load_info_i        ( in_load_info_lo[0]        ),
      .in_mask_i             ( in_mask_lo[0]             ),
      .in_src_x_cord_i       ( in_src_x_cord_lo[0]       ),
      .in_src_y_cord_i       ( in_src_y_cord_lo[0]       ),
      .in_v_i                ( in_v_lo[0]                ),
      .in_we_i               ( in_we_lo[0]               ),
      .in_yumi_o             ( in_yumi_li[0]             ),

      .returning_data_o      ( returning_data_li[0]      ),
      .returning_v_o         ( returning_v_li[0]         ),

      .out_credits_i         ( out_credits_lo[0]         ),
      .out_packet_o          ( out_packet_li[0]          ),
      .out_credit_or_ready_i ( out_credit_or_ready_lo[0] ),
      .out_v_o               ( out_v_li[0]               ),

      .returned_data_r_i     ( returned_data_lo[0]       ),
      .returned_fifo_full_i  ( returned_fifo_full_lo[0]  ),
      .returned_pkt_type_r_i ( returned_pkt_type_lo[0]   ),
      .returned_reg_id_r_i   ( returned_reg_id_lo[0]     ),
      .returned_v_r_i        ( returned_v_lo[0]          ),
      .returned_yumi_o       ( returned_yumi_li[0]       )
    );

    //--------------------------------------------------------------
    // Tie off unused endpoint interfaces
    //--------------------------------------------------------------
    // TODO: support multiple memory masters

    for (genvar i = 1; i < 4; i++) begin
      assign in_yumi_li[i] = in_v_lo[i];
      assign returning_data_li[i] = 0;
      assign returning_v_li[i] = 1'b0;
      assign out_v_li[i] = 1'b0;
      assign out_packet_li[i] = 0;
      assign returned_yumi_li[i] = returned_v_lo[i];
    end

endmodule
