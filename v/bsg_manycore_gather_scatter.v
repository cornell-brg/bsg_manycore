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

    //--------------------------------------------------------------
    //  CSR definitions
     enum {
         CSR_CMD_IDX =0         //command, write to start the transcation
        ,CSR_STATUS_IDX         //the status, 0: idle, 1: running
        ,CSR_SRC_ADDR_IDX       //source addr
        ,CSR_SRC_CORD_IDX       //the source x/y cord, X==15:0, Y=31:16
        ,CSR_DST_ADDR_IDX       //dest addr
        ,CSR_BYTE_LEN_IDX       //legnth in byte
        ,CSR_SIG_ADDR_IDX       //the signal addr, write non-zero to indicates finish
        ,CSR_NUM_lp
    } CSR_INDX;
    
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

   `declare_bsg_manycore_packet_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p, load_id_width_p);
    bsg_manycore_packet_s             out_packet_li     ;
    logic                             out_v_li          ;
    logic                             out_ready_lo      ;

    logic                             in_yumi_li        ;
    logic                             returning_v_r     ;

    logic                             returned_v_lo     ;
    logic[data_width_p-1:0]           returned_data_lo  ;

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
    ,.out_v_i           ( out_v_li                      )
    ,.out_packet_i      ( out_packet_li                 )
    ,.out_ready_o       ( out_ready_lo                  )
   // local returned data interface
   // Like the memory interface, processor should always ready be to
   // handle the returned data
    ,.returned_data_r_o         (returned_data_lo     )
    ,.returned_v_r_o            (returned_v_lo  )
    ,.returned_load_id_r_o      (             )
    ,.returned_fifo_full_o      (             )
    ,.returned_yumi_i           (   1'b0      )

    ,.out_credits_o     (               )
    );

    //--------------------------------------------------------------
    // assign the signals to endpoint
    assign  in_yumi_li  =       in_v_lo   ;     //we can always handle the reqeust

    always_ff@(posedge clk_i)
        if( reset_i ) returning_v_r <= 1'b0;
        else          returning_v_r <= in_yumi_li;

    //--------------------------------------------------------------
    //  The DMA state machine
    typedef enum logic[0:0] {
        eGS_dma_idle    = 1'b0
       ,eGS_dma_busy    = 1'b1
    }GS_dma_stat;

    GS_dma_stat  curr_stat_e_r,  next_stat_e ;
    
    wire dma_run_en =  in_v_lo & ( in_addr_lo == CSR_CMD_IDX );
    wire dma_finish;

    always_comb begin
        case ( curr_stat_e_r )
            eGS_dma_idle :
                if( dma_run_en)         next_stat_e = eGS_dma_busy;
                else                    next_stat_e = eGS_dma_idle;
            eGS_dma_busy :
                if( dma_finish)         next_stat_e = eGS_dma_idle;
                else                    next_stat_e = eGS_dma_busy;
        endcase
    end

    always_ff@( posedge clk_i ) begin
        if( reset_i )  curr_stat_e_r <= eGS_dma_idle;
        else           curr_stat_e_r <= next_stat_e ;
    end
     
    //--------------------------------------------------------------
    //  The Length Counter
     wire finish_one_word       = returned_v_lo;
     wire                       counter_overflow;
     wire [data_width_p-2-1:0]  count_lo;
     bsg_counter_clear_up#( 
                .init_val_p     (0               )
               ,.max_val_p      ( (1<<(data_width_p-2) ) -2  )
     ) run_word_counter (
        .clk_i      ( clk_i                                     )
       ,.reset_i    ( reset_i                                   )
       ,.clear_i    ( counter_overflow                          )
       ,.up_i       ( finish_one_word                           )
       ,.count_o    ( count_lo                                  )
    );
      
    assign counter_overflow = count_lo ==  CSR_mem_r [ CSR_BYTE_LEN_IDX ][data_width_p-1 : 2 ] ;

    assign dma_finish = counter_overflow ;
    //--------------------------------------------------------------
    //  Master interface to load 
    wire   eOp_n            =  `ePacketOp_remote_load   ;
    assign out_v_li         =  curr_stat_e_r == eGS_dma_busy ;
    assign out_packet_li    = '{
                                 addr           :       CSR_mem_r[ CSR_SRC_ADDR_IDX ] >> 2
                                ,op             :       eOp_n
                                ,op_ex          :       {(data_width_p>>3){1'b1}}
                                ,payload        :       'b0 
                                ,src_y_cord     :       my_y_i
                                ,src_x_cord     :       my_x_i
                                ,y_cord         :       y_cord_width_p'( CSR_mem_r[ CSR_SRC_CORD_IDX][31: 16] )
                                ,x_cord         :       x_cord_width_p'( CSR_mem_r[ CSR_SRC_CORD_IDX][15: 0 ] )
                                };
    
    //--------------------------------------------------------------
    // Checking 
    // synopsys translate_off
    always_ff@(negedge clk_i ) begin
        if( in_v_lo &&  (in_addr_lo >= CSR_NUM_lp ) ) begin
                $error("## Invalid CSR addr in Gather/Scatter Module, addr=%h,%t, %m", in_addr_lo, $time);
                $finish();
        end

        if( 1 ) begin
                if( returned_v_lo ) begin
                        $display("## Recieved data = %h, counter = %d", returned_data_lo, count_lo );
                end
                
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
