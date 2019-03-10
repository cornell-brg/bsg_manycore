//====================================================================
// bsg_manycore_gather_scatter.v
// 03/10/2019, shawnless.xie@gmail.com
//====================================================================
// A module that do gather/scatter  
//
`include "bsg_manycore_packet.vh"

module bsg_manycore_gather_scatter#( 
                             x_cord_width_p         = "inv"
                            ,y_cord_width_p         = "inv"
                            ,data_width_p           = 32
                            ,addr_width_p           = 32
                            ,load_id_width_p        = 11
                            ,max_out_credits_p      = 200
                            ,packet_width_lp                = `bsg_manycore_packet_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p, load_id_width_p)
                            ,return_packet_width_lp         = `bsg_manycore_return_packet_width(x_cord_width_p,y_cord_width_p,data_width_p,load_id_width_p)
                            ,bsg_manycore_link_sif_width_lp = `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p, load_id_width_p)
                            ,debug_p                = 1
                            /* Dummy parameter for compatability with socket*/    
                            ,hetero_type_p          = 1
                            ,dmem_size_p            = "inv" 
                            ,epa_byte_addr_width_p  = "inv" 
                            ,dram_ch_addr_width_p   = "inv"
                            ,dram_ch_start_col_p    = "inv"
                            ,icache_entries_p       = "inv"
                            ,icache_tag_width_p     = "inv"
                           )
   (  input clk_i
    , input reset_i

    // mesh network
    , input  [bsg_manycore_link_sif_width_lp-1:0] link_sif_i
    , output [bsg_manycore_link_sif_width_lp-1:0] link_sif_o

    , input   [x_cord_width_p-1:0]                my_x_i
    , input   [y_cord_width_p-1:0]                my_y_i

    // Dummy outputs to be compatilbe with the socket
    , output                                      freeze_o
    );

    localparam CSR_NUM_lp          =       1;
    localparam CSR_TEST_IDX_lp     =       0;

    
    //--------------------------------------------------------------
    // The CSR Memory
    //valid request
    logic [data_width_p-1:0] CSR_mem_r [ CSR_NUM_lp ]           ; 

    logic                               in_v_lo                 ;
    logic[data_width_p-1:0]             in_data_lo              ;
    logic[addr_width_p-1:0]             in_addr_lo              ;
    logic                               in_we_lo                ;
    // write
    always@( posedge clk_i)
        if( in_we_lo & in_v_lo )
                CSR_mem_r[ in_addr_lo ] <=  in_data_lo;

    // read
    logic[data_width_p-1:0]             read_data_r             ;
    always@( posedge clk_i)
        if( ~in_we_lo & in_v_lo)
                read_data_r <= CSR_mem_r[ in_addr_lo ] ;

    //--------------------------------------------------------------
    // instantiate the endpoint standard

    logic                             in_yumi_li        ;
    logic                             returning_v_r     ;
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
    ,.in_mask_o  (                      )
    ,.in_addr_o  ( in_addr_lo           )
    ,.in_we_o    ( in_we_lo             )
    ,.in_src_x_cord_o(  )
    ,.in_src_y_cord_o(  )

    // The memory read value
    ,.returning_data_i  ( read_data_r   )
    ,.returning_v_i     ( returning_v_r )

    // local outgoing data interface (does not include credits)
    // Tied up all the outgoing signals
    ,.out_v_i           ( 1'b0                          )
    ,.out_packet_i      ( packet_width_lp'(0)           )
    ,.out_ready_o       (                               )
   // local returned data interface
   // Like the memory interface, processor should always ready be to
   // handle the returned data
    ,.returned_data_r_o(                )
    ,.returned_v_r_o   (                )
    ,.returned_load_id_r_o(             )
    ,.returned_fifo_full_o(             )
    ,.returned_yumi_i      (   1'b0      )

    ,.out_credits_o     (               )
    );

    //--------------------------------------------------------------
    // assign the signals to endpoint
    assign  in_yumi_li  =       in_v_lo   ;     //we can always handle the reqeust

    always_ff@(posedge clk_i)
        if( reset_i ) returning_v_r <= 1'b0;
        else          returning_v_r <= in_yumi_li;

    //--------------------------------------------------------------
    // Checking 
    // synopsys translate_off
    always_ff@(negedge clk_i ) begin
        if( in_v_lo &&  (in_addr_lo >= CSR_NUM_lp ) ) begin
                $error("## Invalid CSR addr in Gather/Scatter Module, addr=%h,%t, %m", in_addr_lo, $time);
                $finish();
        end
        if( 1 ) begin
                
                if( in_v_lo ) begin
                        if( in_we_lo )
                                $display("## G/S Write: addr=%h, value=%h", in_addr_lo<<2, in_data_lo);  
                        else
                                $display("## G/S Read : addr=%h, value=%h", in_addr_lo<<2, CSR_mem_r[in_addr_lo]);  
                end
        end
    end
    // synopsys translate_on

endmodule
