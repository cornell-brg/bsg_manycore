//=========================================================================
// brg_systolic_xcel_template.v
//=========================================================================
//
// Author : Shunning Jiang
// Date   : Apr 13, 2019

`include "bsg_manycore_packet.vh"
`include "../../alloy-sim/pymtl/build/HBTile_HBIfcDotProductAltPRTL.v"
`include "../../alloy-sim/pymtl/build/HBTile_HBIfcDotProductBasePRTL.v"
`include "../../alloy-sim/pymtl/build/HBIfcColEnginePRTL_0x7d1da687558b8b5a.v"
`include "../../alloy-sim/pymtl/build/HBIfcRowEnginePRTL_0x3cc2ffdadbf4c450.v"

module brg_systolic_xcel_template
#(
  x_cord_width_p         = "inv",
  y_cord_width_p         = "inv",
  dmem_size_p            = "inv",
  data_width_p           = 32,
  addr_width_p           = 32,
  load_id_width_p        = 11,
  max_out_credits_p      = 200,
  packet_width_lp                = `bsg_manycore_packet_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p, load_id_width_p),
  return_packet_width_lp         = `bsg_manycore_return_packet_width(x_cord_width_p,y_cord_width_p,data_width_p,load_id_width_p),
  bsg_manycore_link_sif_width_lp = `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p, load_id_width_p),
  debug_p                = 1,
  /* Dummy parameter for compatability with socket*/
  hetero_type_p          = 2,
  epa_byte_addr_width_p  = "inv",
  dram_ch_addr_width_p   = "inv",
  dram_ch_start_col_p    = "inv",
  icache_entries_p       = "inv",
  icache_tag_width_p     = "inv"
)
(
  input clk_i,
  input reset_i,

  // mesh network
  input  [bsg_manycore_link_sif_width_lp-1:0] link_sif_i,
  output [bsg_manycore_link_sif_width_lp-1:0] link_sif_o,

  // Shunning: systolic network
  input  [37:0] in_row_msg,
  input         in_row_val,
  output        in_row_rdy,
  input  [37:0] in_col_msg,
  input         in_col_val,
  output        in_col_rdy,
  output [37:0] out_row_msg,
  output        out_row_val,
  input         out_row_rdy,
  output [37:0] out_col_msg,
  output        out_col_val,
  input         out_col_rdy,

  input  [x_cord_width_p-1:0]                 my_x_i,
  input  [y_cord_width_p-1:0]                 my_y_i,

  // Dummy outputs to be compatilbe with the socket
  output                                      freeze_o
);

    // Slave:
    // - input is valid/yumi
    // - output is valid

    logic                        in_v_lo;
    logic[data_width_p-1:0]      in_data_lo;
    logic[addr_width_p-1:0]      in_addr_lo;
    logic                        in_we_lo;
    logic[(data_width_p>>3)-1:0] in_mask_lo;
    logic                        in_yumi_li;

    logic                        returning_v_li;
    logic[data_width_p-1:0]      returning_data_li;

    // Master:
    // - output is valid/ready
    // - input is valid(/yumi)

   `declare_bsg_manycore_packet_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p, load_id_width_p);
    bsg_manycore_packet_s             out_packet_li     ;
    logic                             out_v_li          ;
    logic                             out_ready_lo      ;

    logic [load_id_width_p-1:0] returned_load_id_r_lo;
    logic [data_width_p-1:0]    returned_data_lo  ;
    logic                       returned_v_lo     ;
    //logic                       returned_fifo_full_lo;
    //logic                       returned_yumi_li;

    logic[$clog2(max_out_credits_p+1)-1:0] out_credits_lo;

    // wires from accelerator's master interface
    logic [32-1:0]                xcel_master_addr;
    logic [data_width_p-1:0]      xcel_master_data;
    logic [(data_width_p>>3)-1:0] xcel_master_mask;
    logic                         xcel_master_type;
    logic [load_id_width_p-1:0]   xcel_master_opq;

    // determine master_data based pm xcel_master_type
    bsg_manycore_packet_payload_u   master_data;

    // Shunning: We use accelerator's req opaque fields only for loads.
    // We need to encode the loads into data
    always_comb begin
      if (xcel_master_type == `ePacketOp_remote_load)
        master_data.load_info_s.load_id = xcel_master_opq;
      else
        master_data = xcel_master_data;
    end

    //--------------------------------------------------------------
    // instantiate the endpoint standard
    //--------------------------------------------------------------

    bsg_manycore_endpoint_standard  #(
                              .x_cord_width_p        ( x_cord_width_p    )
                             ,.y_cord_width_p        ( y_cord_width_p    )
                             ,.fifo_els_p            ( 4                 )
                             ,.data_width_p          ( data_width_p      )
                             ,.addr_width_p          ( addr_width_p      )
                             ,.load_id_width_p       ( load_id_width_p   )
                             ,.max_out_credits_p     ( max_out_credits_p )
                        )endpoint_gs

   ( .clk_i
    ,.reset_i

    // mesh network
    ,.link_sif_i
    ,.link_sif_o
    ,.my_x_i
    ,.my_y_i

    // slave request -- local incoming data interface
    ,.in_v_o     ( in_v_lo    )
    ,.in_yumi_i  ( in_yumi_li )
    ,.in_data_o  ( in_data_lo )
    ,.in_mask_o  ( in_mask_lo )
    ,.in_addr_o  ( in_addr_lo )
    ,.in_we_o    ( in_we_lo   )
    ,.in_src_x_cord_o(  )
    ,.in_src_y_cord_o(  )

    // slave response
    ,.returning_data_i  ( returning_data_li )
    ,.returning_v_i     ( returning_v_li )

    // master request -- local outgoing data interface
    ,.out_v_i           ( out_v_li      )
    ,.out_packet_i      ( out_packet_li )
    ,.out_ready_o       ( out_ready_lo  )

    // master response -- local returned data interface
    ,.returned_data_r_o    ( returned_data_lo    )
    ,.returned_load_id_r_o ( returned_load_id_r_lo )
    ,.returned_v_r_o       ( returned_v_lo       )
    ,.returned_fifo_full_o (  ) // No need to check fifo full
    // Shunning: assume accelerator is always ready to accept response
    ,.returned_yumi_i      ( returned_v_lo      )
    ,.out_credits_o ( out_credits_lo )
    );

    bsg_manycore_pkt_encode #(.x_cord_width_p (x_cord_width_p)
                             ,.y_cord_width_p(y_cord_width_p)
                             ,.data_width_p (data_width_p )
                             ,.addr_width_p (addr_width_p )
                             ,.epa_word_addr_width_p( epa_byte_addr_width_p -2 )
                             ,.dram_ch_addr_width_p ( dram_ch_addr_width_p)
                             ,.dram_ch_start_col_p  ( dram_ch_start_col_p )
                             ) pkt_encode
     (.clk_i(clk_i)

      // the memory request, from the core's data memory port
      ,.v_i       ( out_v_li  )
      ,.data_i    ( master_data )
      ,.addr_i    ( xcel_master_addr )
      ,.we_i      ( xcel_master_type )
      ,.swap_aq_i ( 1'b0 ) // Shunning: we don't issue AMO in xcel
      ,.swap_rl_i ( 1'b0 )
      ,.mask_i    ( xcel_master_mask )
      ,.tile_group_x_i ( x_cord_width_p'(0) ) // Shunning: we assume the Makefile always specify 0
      ,.tile_group_y_i ( y_cord_width_p'(1) ) //           and 1 for these two.
      ,.my_x_i    (my_x_i)
      ,.my_y_i    (my_y_i)

      // directly out to the network!
      ,.v_o    (out_request)
      ,.data_o (out_packet_li)
      );

    // Instantiate brg xcel

`define BRG_XCEL_SELECT(XCEL_TYPE,XCEL_MODULE) \
  if (hetero_type_p == (XCEL_TYPE)) \
    begin: h \
      XCEL_MODULE xcel \
      ( \
         .clk        ( clk_i      ), \
         .reset      ( reset_i    ), \
         .slave_addr ( in_addr_lo ), \
         .slave_data ( in_data_lo ), \
         .slave_mask ( in_mask_lo  ), \
         .slave_type ( in_we_lo    ), \
         .slave_val  ( in_v_lo     ), \
         .slave_yum  ( in_yumi_li  ), \
         .slave_ret_data ( returning_data_li ), \
         .slave_ret_val  ( returning_v_li    ), \
         .master_val  ( out_v_li ), \
         .master_type ( xcel_master_type ), \
         .master_addr ( xcel_master_addr ), \
         .master_opq  ( xcel_master_opq  ), \
         .master_data ( xcel_master_data ), \
         .master_mask ( xcel_master_mask ), \
         .master_rdy  ( out_ready_lo ), \
         .master_ret_data( returned_data_lo  ), \
         .master_ret_opq ( returned_load_id_r_lo ), \
         .master_ret_val ( returned_v_lo ), \
         .in_row_msg (in_row_msg), \
         .in_row_val (in_row_val), \
         .in_row_rdy (in_row_rdy), \
         .in_col_msg (in_col_msg), \
         .in_col_val (in_col_val), \
         .in_col_rdy (in_col_rdy), \
         .out_row_msg (out_row_msg), \
         .out_row_val (out_row_val), \
         .out_row_rdy (out_row_rdy), \
         .out_col_msg (out_col_msg), \
         .out_col_val (out_col_val), \
         .out_col_rdy (out_col_rdy) \
      ); \
    end

   // add as many types as you like...
   // Shunning: !!! We can only instantiate those who instantiate
   //           brg_systolic_xcel_template in bsg_manycore_hetero_socket.v

   `BRG_XCEL_SELECT(300,HBIfcColEnginePRTL_0x7d1da687558b8b5a) else // brg column engine
   `BRG_XCEL_SELECT(301,HBIfcRowEnginePRTL_0x3cc2ffdadbf4c450) else // brg row engine
   `BRG_XCEL_SELECT(302,HBTile_HBIfcDotProductBasePRTL) else // brg dense xcel base
   `BRG_XCEL_SELECT(303,HBTile_HBIfcDotProductAltPRTL) else // brg dense xcel alt
     begin : nh
	// synopsys translate_off
        initial
          begin
             $error("## unidentified brg systolic accelerator type",hetero_type_p);
             $finish();
          end
        // synopsys translate_on
     end

    //--------------------------------------------------------------
    // Checking
    // synopsys translate_off
    if (debug_p)
      always_ff@(negedge clk_i ) begin
        if (out_v_li && out_ready_lo)
          $display("MASTER (%d,%d) send mem req  %d %h %h", my_x_i, my_y_i, xcel_master_type, xcel_master_addr, xcel_master_data);
        if (returned_v_lo)
          $display("MASTER (%d,%d) recv mem resp %d %h, credit: %d", my_x_i, my_y_i, returned_load_id_r_lo, returned_data_lo, out_credits_lo);

        if( in_yumi_li ) begin
          if ( in_we_lo )
            $display("SLAVE  (%d,%d) CSR Write: csr=%h, value=%h", my_x_i, my_y_i, in_addr_lo, in_data_lo);
          else
            $display("SLAVE  (%d,%d) CSR Read : csr=%h", my_x_i, my_y_i, in_addr_lo);
        end

        if (in_row_val && in_row_rdy)
          $display("ROW    (%d,%d) recv %d", my_x_i, my_y_i, in_row_msg[31:0]);
        if (in_col_val && in_col_rdy)
          $display("COL    (%d,%d) recv %d", my_x_i, my_y_i, in_col_msg[31:0]);

      end

    // synopsys translate_on

endmodule
