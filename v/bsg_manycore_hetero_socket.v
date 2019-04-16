// MBT 9/13/16
//
//  THIS IS A TEMPLATE THAT YOU CUSTOMIZE FOR YOUR HETERO MANYCORE
//
//  Edit the lines:
//
//  `HETERO_TYPE_MACRO(1,bsg_accelerator_add)
//
//  by replacing bsg_accelerator_add with your core's name
//
//  then change the makefile to use your modified file instead of
//  this one.
//

`include "bsg_manycore_packet.vh"
`include "brg_slave_xcel_template.v"
`include "brg_xcel_template.v"
`include "brg_systolic_xcel_template.v"

`ifdef bsg_FPU
`include "float_definitions.vh"
`endif

// Shunning: we enhance this to pass through systolic network for
// non-systolic accelerators
`define HETERO_TYPE_MACRO(BMC_TYPE,BMC_TYPE_MODULE)             \
   if (hetero_type_p == (BMC_TYPE))                             \
     if ( hetero_type_p < 300 )                                 \
     begin: h                                                   \
        BMC_TYPE_MODULE #(.x_cord_width_p(x_cord_width_p)       \
                          ,.y_cord_width_p(y_cord_width_p)      \
                          ,.data_width_p(data_width_p)          \
                          ,.addr_width_p(addr_width_p)          \
                          ,.load_id_width_p(load_id_width_p)    \
                          ,.dmem_size_p (dmem_size_p )          \
                          ,.epa_byte_addr_width_p(epa_byte_addr_width_p)  \
                          ,.dram_ch_addr_width_p ( dram_ch_addr_width_p )  \
                          ,.dram_ch_start_col_p  ( dram_ch_start_col_p) \
                          ,.debug_p(debug_p)                    \
			  ,.icache_entries_p(icache_entries_p)            \
                          ,.icache_tag_width_p (icache_tag_width_p) \
            		  ,.max_out_credits_p(max_out_credits_p)\
                          ,.hetero_type_p(hetero_type_p)        \
                          ) z                                   \
          (.clk_i                                               \
           ,.reset_i                                            \
           ,.link_sif_i                                         \
           ,.link_sif_o                                         \
           ,.my_x_i                                             \
           ,.my_y_i                                             \
           ,.freeze_o                                           \
 `ifdef bsg_FPU                                                 \
           ,.fam_out_s_i                                        \
           ,.fam_in_s_o                                         \
 `endif                                                         \
           );                                                   \
        assign out_row_msg = in_row_msg;                        \
        assign out_row_val = in_row_val;                        \
        assign in_row_rdy = out_row_rdy;                        \
        assign out_row_msg = in_row_msg;                        \
        assign out_row_val = in_row_val;                        \
        assign in_row_rdy = out_row_rdy;                        \
     end                                                        \
     else begin                                                 \
        BMC_TYPE_MODULE #(.x_cord_width_p(x_cord_width_p)       \
                          ,.y_cord_width_p(y_cord_width_p)      \
                          ,.data_width_p(data_width_p)          \
                          ,.addr_width_p(addr_width_p)          \
                          ,.load_id_width_p(load_id_width_p)    \
                          ,.dmem_size_p (dmem_size_p )          \
                          ,.epa_byte_addr_width_p(epa_byte_addr_width_p)  \
                          ,.dram_ch_addr_width_p ( dram_ch_addr_width_p )  \
                          ,.dram_ch_start_col_p  ( dram_ch_start_col_p) \
                          ,.debug_p(debug_p)                    \
                          ,.icache_entries_p(icache_entries_p)  \
                          ,.icache_tag_width_p (icache_tag_width_p) \
                          ,.max_out_credits_p(max_out_credits_p)\
                          ,.hetero_type_p(hetero_type_p)        \
                          ) z                                   \
          (.clk_i                                               \
           ,.reset_i                                            \
           ,.link_sif_i                                         \
           ,.link_sif_o                                         \
           ,.my_x_i                                             \
           ,.my_y_i                                             \
           ,.freeze_o                                           \
           ,.in_row_msg (in_row_msg), \
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
 `ifdef bsg_FPU                                                 \
           ,.fam_out_s_i                                        \
           ,.fam_in_s_o                                         \
 `endif                                                         \
           );                                                   \
     end

module bsg_manycore_hetero_socket #(  x_cord_width_p      = "inv"
                                    , y_cord_width_p    = "inv"
                                    , data_width_p      = 32
                                    , addr_width_p      = "inv"
                                    , load_id_width_p   = 5
                                    , dmem_size_p       = "inv"
                                    , epa_byte_addr_width_p  = "inv"
                                    , dram_ch_addr_width_p = "inv"
                                    , dram_ch_start_col_p = 0
                                    , debug_p           = 0
				    , icache_entries_p       = "inv" // in words
                                    , icache_tag_width_p= "inv"
                	            , max_out_credits_p = 200
                                    , hetero_type_p     = 1
                                    , bsg_manycore_link_sif_width_lp = `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p,load_id_width_p)
                                    )
   (  input   clk_i
    , input reset_i

    // input and output links
    , input  [bsg_manycore_link_sif_width_lp-1:0] link_sif_i
    , output [bsg_manycore_link_sif_width_lp-1:0] link_sif_o

    // Shunning: systolic network
   ,input  [37:0] in_row_msg,
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
    input         out_col_rdy

    // tile coordinates
    , input   [x_cord_width_p-1:0]                my_x_i
    , input   [y_cord_width_p-1:0]                my_y_i
    , output logic freeze_o

 `ifdef bsg_FPU
    , input  f_fam_out_s                         fam_out_s_i
    , output f_fam_in_s                          fam_in_s_o
 `endif

    );

   // add as many types as you like...
   `HETERO_TYPE_MACRO(0,bsg_manycore_proc_vanilla) else
   `HETERO_TYPE_MACRO(1,bsg_manycore_gather_scatter) else

   `HETERO_TYPE_MACRO(100, brg_slave_xcel_template) else  // brg gcd xcel
   `HETERO_TYPE_MACRO(200, brg_xcel_template) else // brg simple xcel

   `HETERO_TYPE_MACRO(300, brg_systolic_xcel_template) else // brg column engine
   `HETERO_TYPE_MACRO(301, brg_systolic_xcel_template) else // brg row engine
   `HETERO_TYPE_MACRO(302, brg_systolic_xcel_template) else // brg dense xcel base
   `HETERO_TYPE_MACRO(303, brg_systolic_xcel_template) else // brg dense xcel alt
     begin : nh
	// synopsys translate_off
        initial
          begin
             $error("## unidentified hetero core type ",hetero_type_p);
             $finish();
          end
        // synopsys translate_on
     end

endmodule
