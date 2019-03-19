//=========================================================================
// brg_slave_xcel_template.v
//=========================================================================
//
// Author : Shunning Jiang
// Date   : Mar 15, 2019

`include "bsg_manycore_packet.vh"
`include "HBIfcGcdXcelPRTL_0xeef3ea699c5ed9a.v"

module brg_slave_xcel_template
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
    // - input is valid

   `declare_bsg_manycore_packet_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p, load_id_width_p);
    bsg_manycore_packet_s             out_packet_li     ;
    logic                             out_v_li          ;
    logic                             out_ready_lo      ;

    logic                             returned_v_lo     ;
    logic[data_width_p-1:0]           returned_data_lo  ;
    logic[load_id_width_p-1:0]        returned_load_id_r_lo, load_id_r;
    // Note that this out_credits_lo should be modified by accelerators that
    // uses master memory interface
    logic[$clog2(max_out_credits_p+1)-1:0] out_credits_lo;

    //--------------------------------------------------------------
    // instantiate the endpoint standard
    //--------------------------------------------------------------
    // Shunning: since this is a slave xcel template, we don't use master
    // side of things

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

    // local incoming data interface
    ,.in_v_o     ( in_v_lo              )
    ,.in_yumi_i  ( in_yumi_li           )
    ,.in_data_o  ( in_data_lo           )
    ,.in_mask_o  ( in_mask_lo           )
    ,.in_addr_o  ( in_addr_lo           )
    ,.in_we_o    ( in_we_lo             )
    ,.in_src_x_cord_o(  )
    ,.in_src_y_cord_o(  )

    // slave response
    ,.returning_data_i  ( returning_data_li )
    ,.returning_v_i     ( returning_v_li )

    // Shunning: we need to connect specific values to some ports to
    //           prevent X similar to the following link.
    // https://bitbucket.org/taylor-bsg/bsg_manycore/src/ff0c06906e27f8080dcab0ca2c12fa6b95cddc86/testbenches/mesh_example/mesh_slave_example.v#lines-90:104

    // local outgoing data interface (does not include credits)
    // Tied up all the outgoing signals
    ,.out_v_i           ( 1'b0                      )
    ,.out_packet_i      ( packet_width_lp'(0)                 )
    ,.out_ready_o       (                   )
    // local returned data interface
    // Like the memory interface, processor should always ready be to
    // handle the returned data
    ,.returned_data_r_o         (     )
    ,.returned_v_r_o            (        )
    ,.returned_load_id_r_o      ()
    ,.returned_fifo_full_o      (             )
    ,.returned_yumi_i           (1'b0)

    ,.out_credits_o     (               )
    );

    // Instantiate brg xcel

    HBIfcGcdXcelPRTL_0xeef3ea699c5ed9a xcel
    (
       .clk            ( clk_i      ),
       .reset          ( reset_i    ),
       .slave_addr     ( in_addr_lo ),
       .slave_data     ( in_data_lo ),
       .slave_mask     ( in_mask_lo  ),
       .slave_type     ( in_we_lo    ),
       .slave_val      ( in_v_lo     ),
       .slave_yum      ( in_yumi_li  ),

       .slave_ret_data ( returning_data_li ),
       .slave_ret_val  ( returning_v_li    )
    );

    //--------------------------------------------------------------
    // Checking
    // synopsys translate_off
    always_ff@(negedge clk_i ) begin

      if( in_yumi_li ) begin
        if ( in_we_lo )
          $display("## (%d,%d) G/S CSR Write: csr=%h, value=%h", my_x_i, my_y_i, in_addr_lo, in_data_lo);
        else
          $display("## (%d,%d) G/S CSR Read : csr=%h", my_x_i, my_y_i, in_addr_lo);
      end
    end
    // synopsys translate_on

endmodule
