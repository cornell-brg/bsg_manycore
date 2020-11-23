//====================================================================
// brg_vvadd_xcel.v
// Author : Peitian Pan
// Date   : Feb 20, 2020
//====================================================================
// A module that performs vvadd.

module brg_vvadd_xcel
  import bsg_manycore_pkg::*;
  import brg_vvadd_xcel_pkg::*;
  #( 
     x_cord_width_p               = "inv"
    ,y_cord_width_p               = "inv"
    ,data_width_p                 = "inv"
    ,addr_width_p                 = "inv"
    ,dmem_size_p                  = "inv"
    ,vcache_size_p                = "inv"
    ,vcache_block_size_in_words_p = "inv"
    ,vcache_sets_p                = "inv"

    ,mc_composition_p             = "inv"

    ,icache_entries_p             = "inv"
    ,icache_tag_width_p           = "inv"

    ,max_out_credits_p            = 32

    ,num_tiles_x_p                = "inv"
    ,num_tiles_y_p                = "inv"

    /* Dummy parameter for compatability with socket*/
    ,debug_p                      = 1
    ,branch_trace_en_p            = 0

    // Derived local parameters
    ,packet_width_lp                = `bsg_manycore_packet_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
    ,return_packet_width_lp         = `bsg_manycore_return_packet_width(x_cord_width_p,y_cord_width_p,data_width_p)
    ,bsg_manycore_link_sif_width_lp = `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
  )
  (   input clk_i
    , input reset_i

    // mesh network
    , input  [bsg_manycore_link_sif_width_lp-1:0] link_sif_i
    , output [bsg_manycore_link_sif_width_lp-1:0] link_sif_o

    , input   [x_cord_width_p-1:0]                my_x_i
    , input   [y_cord_width_p-1:0]                my_y_i

    // Dummy outputs to be compatilbe with the socket
    // , output                                      freeze_o
    );

    //--------------------------------------------------------------
    // Endpoint standard, RX, and TX
    //--------------------------------------------------------------

    `declare_bsg_manycore_packet_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p);

    // endpoint slave interface ports
    logic                             in_v_lo         ;
    logic[data_width_p-1:0]           in_data_lo      ;
    logic[(data_width_p>>3)-1:0]      in_mask_lo      ;
    logic[addr_width_p-1:0]           in_addr_lo      ;
    logic                             in_we_lo        ;
    bsg_manycore_load_info_s          in_load_info_lo ;
    logic[x_cord_width_p-1:0]         in_src_x_cord_lo;
    logic[y_cord_width_p-1:0]         in_src_y_cord_lo;
    logic                             in_yumi_li      ;

    logic[data_width_p-1:0]           returning_data_li;
    logic                             returning_v_li   ;

    // endpoint master interface ports
    logic                                  out_v_li      ;
    logic[packet_width_lp-1:0]             out_packet_li ;
    logic                                  out_ready_lo  ;
    logic[$clog2(max_out_credits_p+1)-1:0] out_credits_lo;

    logic[data_width_p-1:0]           returned_data_lo     ;
    logic[4:0]                        returned_reg_id_lo   ;
    logic                             returned_v_lo        ;
    bsg_manycore_return_packet_type_e returned_pkt_type_lo ;
    logic                             returned_yumi_li     ;
    logic                             returned_fifo_full_lo;

    bsg_manycore_endpoint_standard
    #(
       .x_cord_width_p        ( x_cord_width_p    )
      ,.y_cord_width_p        ( y_cord_width_p    )
      ,.fifo_els_p            ( 4                 )
      ,.data_width_p          ( data_width_p      )
      ,.addr_width_p          ( addr_width_p      )
      ,.max_out_credits_p     ( max_out_credits_p )
    ) endpoint_gs
    (  .clk_i
      ,.reset_i

      // mesh network
      ,.link_sif_i
      ,.link_sif_o
      ,.my_x_i
      ,.my_y_i

      // slave request
      ,.in_v_o                ( in_v_lo              )
      ,.in_data_o             ( in_data_lo           )
      ,.in_mask_o             ( in_mask_lo           )
      ,.in_addr_o             ( in_addr_lo           )
      ,.in_we_o               ( in_we_lo             )
      ,.in_load_info_o        ( in_load_info_lo      )
      ,.in_src_x_cord_o       ( in_src_x_cord_lo     )
      ,.in_src_y_cord_o       ( in_src_y_cord_lo     )
      ,.in_yumi_i             ( in_yumi_li           )

      // slave response
      ,.returning_data_i      ( returning_data_li    )
      ,.returning_v_i         ( returning_v_li       )

      // local outgoing data interface (does not include credits)
      // Tied up all the outgoing signals
      // master request
      ,.out_v_i               ( out_v_li             )
      ,.out_packet_i          ( out_packet_li        )
      ,.out_credit_or_ready_o ( out_ready_lo         )
      ,.out_credits_o         ( out_credits_lo       )

      // local returned data interface
      // Like the memory interface, processor should always ready be to
      // handle the returned data
      // master reponse
      ,.returned_data_r_o     ( returned_data_lo      )
      ,.returned_reg_id_r_o   ( returned_reg_id_lo    )
      ,.returned_v_r_o        ( returned_v_lo         )
      ,.returned_pkt_type_r_o ( returned_pkt_type_lo  ) // new field, do not use it for now
      ,.returned_yumi_i       ( returned_yumi_li      )
      ,.returned_fifo_full_o  ( returned_fifo_full_lo )
    );

    // RX ports
    logic rx_is_CSR_lo;
    logic rx_is_local_mem_lo;
    logic rx_we_lo;
    logic [addr_width_p-1:0] rx_addr_lo;
    logic [data_width_p-1:0] rx_wdata_lo;
    logic [(data_width_p>>3)-1:0] rx_mask_lo;
    logic rx_v_lo;
    logic rx_yumi_li;
    logic [data_width_p-1:0] rx_returning_data_li;
    logic rx_returning_v_li;

    brg_vvadd_xcel_network_rx
    #(
       .x_cord_width_p        ( x_cord_width_p        )
      ,.y_cord_width_p        ( y_cord_width_p        )
      ,.dmem_size_p           ( dmem_size_p           )
      ,.data_width_p          ( data_width_p          )
      ,.addr_width_p          ( addr_width_p          )
    ) rx
    (
       .clk_i
      ,.reset_i
      ,.my_x_i
      ,.my_y_i

      // endpoint side - slave request
      ,.v_i                   ( in_v_lo              )
      ,.data_i                ( in_data_lo           )
      ,.mask_i                ( in_mask_lo           )
      ,.addr_i                ( in_addr_lo           )
      ,.we_i                  ( in_we_lo             )
      ,.load_info_i           ( in_load_info_lo      )
      ,.src_x_cord_i          ( in_src_x_cord_lo     )
      ,.src_y_cord_i          ( in_src_y_cord_lo     )
      ,.yumi_o                ( in_yumi_li           )

      // endpoint side - slave response
      ,.returning_data_o      ( returning_data_li    )
      ,.returning_v_o         ( returning_v_li       )

      // xcel side
      ,.rx_is_CSR_o           ( rx_is_CSR_lo       )
      ,.rx_is_local_mem_o     ( rx_is_local_mem_lo )
      ,.rx_we_o               ( rx_we_lo           )
      ,.rx_addr_o             ( rx_addr_lo         )
      ,.rx_wdata_o            ( rx_wdata_lo        )
      ,.rx_mask_o             ( rx_mask_lo         )
      ,.rx_v_o                ( rx_v_lo            )
      ,.rx_yumi_i             ( rx_yumi_li         )

      ,.rx_returning_data_i   ( rx_returning_data_li )
      ,.rx_returning_v_i      ( rx_returning_v_li    )
    );

    // TX ports
    logic tx_v_li, tx_ready_lo, tx_fetching_li, tx_returned_v_lo;
    logic [addr_width_p-1:0] tx_addr_li, tx_signal_addr_li;
    logic [data_width_p-1:0] tx_returned_data_lo;
    logic [4:0] tx_reg_id_li, tx_returned_reg_id_lo;
    logic [$clog2(max_out_credits_p+1)-1:0] tx_credits_lo;

    brg_vvadd_xcel_network_tx
    #(
       .x_cord_width_p        ( x_cord_width_p        )
      ,.y_cord_width_p        ( y_cord_width_p        )
      ,.data_width_p          ( data_width_p          )
      ,.addr_width_p          ( addr_width_p          )
      ,.mc_composition_p      ( mc_composition_p      )
    ) tx
    (
       .clk_i
      ,.reset_i
      ,.my_x_i
      ,.my_y_i

      // endpoint side - master request
      ,.v_o                   ( out_v_li              )
      ,.packet_o              ( out_packet_li         )
      ,.ready_i               ( out_ready_lo          )
      ,.credits_i             ( out_credits_lo        )

      // endpoint side - master response
      ,.returned_data_i       ( returned_data_lo      )
      ,.returned_reg_id_i     ( returned_reg_id_lo    )
      ,.returned_v_i          ( returned_v_lo         )
      ,.returned_pkt_type_i   ( returned_pkt_type_lo  )
      ,.returned_yumi_o       ( returned_yumi_li      )
      ,.returned_fifo_full_i  ( returned_fifo_full_lo )

      // xcel side
      ,.tx_v_i                ( tx_v_li               )
      ,.tx_fetching_i         ( tx_fetching_li        )
      ,.tx_addr_i             ( tx_addr_li            )
      ,.tx_signal_addr_i      ( tx_signal_addr_li     )
      ,.tx_reg_id_i           ( tx_reg_id_li          )
      ,.tx_ready_o            ( tx_ready_lo           )
      ,.tx_credits_o          ( tx_credits_lo         )

      ,.tx_returned_data_o    ( tx_returned_data_lo   )
      ,.tx_returned_reg_id_o  ( tx_returned_reg_id_lo )
      ,.tx_returned_v_o       ( tx_returned_v_lo      )
    );

    //------------------------------------------------------------------
    // Local memory
    //------------------------------------------------------------------

    // For simplicity we instantiated a dual-port SRAM here. One port is
    // dedicated to receiving the data returned from the master interface
    // (mem_xcel_port_lp). The other is used to handle slave interface
    // memory request (mem_proc_port_lp).
    // For the xcel port, the address was based on the configurable
    // DST_ADDR CSR. The device code should make sure that when the xcel
    // has finished, it should read from the same address out of the xcel
    // as it configured the DST_ADDR CSR.
    // Based on the state of the main FSM, the xcel port is used to write
    // the loaded data, read out the operand, and write back the sum.

    localparam mem_addr_width_lp = $clog2(dmem_size_p);
    localparam mem_proc_port_lp  = 0;
    localparam mem_xcel_port_lp  = 1;

    logic [1:0]                           mem_v_li, mem_we_li, mem_yumi_lo, mem_v_lo;
    logic [1:0] [mem_addr_width_lp-1 : 0] mem_addr_li;
    logic [1:0] [data_width_p-1      : 0] mem_data_li, mem_data_lo;
    logic [1:0] [(data_width_p>>3)-1 : 0] mem_mask_li;

    bsg_mem_banked_crossbar
    #(
       .num_ports_p  (2)
      ,.num_banks_p  (1)
      ,.bank_size_p  (dmem_size_p )
      ,.data_width_p (data_width_p)
      // Priority,  0 = fixed hi,
      ,.rr_lo_hi_p   (0) 
    ) mem
    (  .clk_i
      ,.reset_i
      // deprecated, tied to zero
      ,.reverse_pr_i( 1'b0)
      ,.v_i   (mem_v_li)
      ,.w_i   (mem_we_li)
      ,.addr_i(mem_addr_li)
      ,.data_i(mem_data_li)
      ,.mask_i(mem_mask_li)

      // whether the crossbar accepts the input
      ,.yumi_o(mem_yumi_lo)
      ,.v_o   (mem_v_lo   )
      ,.data_o(mem_data_lo)
    );

    //--------------------------------------------------------------
    // CSR Memory
    //--------------------------------------------------------------

    // The protocol requires that there must be an edge between the
    // request handshake and the response valid signal.

    logic [data_width_p-1:0] CSR_mem_r [ CSR_NUM_lp ];
    logic [data_width_p-1:0] CSR_returning_data_r;
    logic CSR_returning_v_r;

    // Write CSR
    always_ff@(posedge clk_i) begin
      if( rx_v_lo & rx_we_lo & rx_is_CSR_lo )
        CSR_mem_r[rx_addr_lo] <= rx_wdata_lo;
    end

    // CSR response message
    always_ff@(posedge clk_i) begin
      if( reset_i )
        CSR_returning_data_r <= '0;
      else if( rx_v_lo & ~rx_we_lo & rx_is_CSR_lo )
        CSR_returning_data_r <= CSR_mem_r[rx_addr_lo];
    end

    // CSR response valid
    always_ff@(posedge clk_i) begin
      if( reset_i )
        CSR_returning_v_r <= '0;
      else
        CSR_returning_v_r <= rx_v_lo & rx_is_CSR_lo;
    end

    logic [data_width_p-1:0] num_elements;
    logic [mem_addr_width_lp-1:0] xcel_base_addr;
    logic [mem_addr_width_lp-1:0] xcel_A_base_addr;
    logic [mem_addr_width_lp-1:0] xcel_B_base_addr;
    Norm_NPA_s addr_a;
    Norm_NPA_s addr_b;
    Norm_NPA_s addr_sig;

    assign num_elements = CSR_mem_r[CSR_NELEM_IDX];
    assign xcel_base_addr = CSR_mem_r[CSR_DST_ADDR_IDX][2+:mem_addr_width_lp];
    assign xcel_A_base_addr = xcel_base_addr;
    assign xcel_B_base_addr = xcel_base_addr + (mem_addr_width_lp)'(num_elements);
    assign addr_a = {CSR_mem_r[CSR_A_ADDR_HI_IDX], CSR_mem_r[CSR_A_ADDR_LO_IDX]};
    assign addr_b = {CSR_mem_r[CSR_B_ADDR_HI_IDX], CSR_mem_r[CSR_B_ADDR_LO_IDX]};
    assign addr_sig = {CSR_mem_r[CSR_SIG_ADDR_HI_IDX], CSR_mem_r[CSR_SIG_ADDR_LO_IDX]};

    //--------------------------------------------------------------
    //  Main FSM
    //--------------------------------------------------------------
    // Ignoring flow control so we can send out all requests at once.

    typedef enum logic[3:0] {
        xcel_idle        = 4'h0
       ,xcel_load_A_send = 4'h1
       ,xcel_load_A_wait = 4'h2
       ,xcel_load_B_send = 4'h3
       ,xcel_load_B_wait = 4'h4
       ,xcel_read_a      = 4'h5
       ,xcel_read_b      = 4'h6
       ,xcel_wb_sum      = 4'h7
       ,xcel_signal      = 4'h8
       ,xcel_wait        = 4'h9
    } Xcel_state_e;

    Xcel_state_e xcel_state_r, xcel_next_state;

    // conditions

    wire xcel_go,
         xcel_all_A_req_sent, xcel_all_A_resp_back,
         xcel_all_B_req_sent, xcel_all_B_resp_back,
         xcel_read_a_finished, xcel_read_b_finished, xcel_wb_sum_finished,
         xcel_last_wb, xcel_signal_sent, xcel_signal_acked;

    // next xcel state

    always_comb begin
      case( xcel_state_r )
        xcel_idle: begin
          if( xcel_go )              xcel_next_state = xcel_load_A_send;
          else                       xcel_next_state = xcel_idle;
        end
        xcel_load_A_send: begin
          if( xcel_all_A_req_sent )  xcel_next_state = xcel_load_A_wait;
          else                       xcel_next_state = xcel_load_A_send;
        end
        xcel_load_A_wait: begin
          if( xcel_all_A_resp_back ) xcel_next_state = xcel_load_B_send;
          else                       xcel_next_state = xcel_load_A_wait;
        end
        xcel_load_B_send: begin
          if( xcel_all_B_req_sent )  xcel_next_state = xcel_load_B_wait;
          else                       xcel_next_state = xcel_load_B_send;
        end
        xcel_load_B_wait: begin
          if( xcel_all_B_resp_back ) xcel_next_state = xcel_read_a;
          else                       xcel_next_state = xcel_load_B_wait;
        end
        xcel_read_a: begin
          if( xcel_read_a_finished ) xcel_next_state = xcel_read_b;
          else                       xcel_next_state = xcel_read_a;
        end
        xcel_read_b: begin
          if( xcel_read_b_finished ) xcel_next_state = xcel_wb_sum;
          else                       xcel_next_state = xcel_read_b;
        end
        xcel_wb_sum: begin
          if( xcel_wb_sum_finished ) begin
            if( xcel_last_wb )       xcel_next_state = xcel_signal;
            else                     xcel_next_state = xcel_read_a;
          end
          else                       xcel_next_state = xcel_wb_sum;
        end
        xcel_signal: begin
          if( xcel_signal_sent )     xcel_next_state = xcel_wait;
          else                       xcel_next_state = xcel_signal;
        end
        xcel_wait: begin
          if( xcel_signal_acked )    xcel_next_state = xcel_idle;
          else                       xcel_next_state = xcel_wait;
        end
        default: begin
                                     xcel_next_state = xcel_idle;
        end
      endcase
    end

    // main FSM

    always_ff@(posedge clk_i) begin
      if( reset_i ) xcel_state_r <= xcel_idle;
      else          xcel_state_r <= xcel_next_state;
    end

    // conditions

    logic [4:0] reg_id_r, last_A_id, last_B_id;
    wire is_rx_writing_go_CSR, xcel_mem_port_go, xcel_master_sent;

    assign last_A_id = 5'(num_elements) - 5'b1;
    assign last_B_id = (5'(num_elements) << 1'b1) - 5'b1;

    assign is_rx_writing_go_CSR = rx_v_lo & rx_yumi_li & rx_we_lo
                              & rx_is_CSR_lo & (rx_addr_lo == CSR_CMD_IDX);
    assign xcel_mem_port_go = mem_v_li[mem_xcel_port_lp] & mem_yumi_lo[mem_xcel_port_lp];
    assign xcel_master_sent = tx_v_li & tx_ready_lo;

    logic [addr_width_p-1:0] load_a_reg_id_r, load_b_reg_id_r;
    logic [data_width_p-1:0] value_a_r, value_b;
    logic [mem_addr_width_lp-1:0] read_a_reg_id_r, read_b_reg_id_r, wb_reg_id_r;
    // Local SRAM has one cycle latency. The output data should be registered
    // one cycle after the handshake succeeds.
    logic read_a_en_r;

    always_ff@(posedge clk_i) begin
      if( reset_i )               reg_id_r <= 'b0;
      else if( xcel_master_sent ) reg_id_r <= reg_id_r + 1'b1;
    end

    assign xcel_go = (xcel_state_r == xcel_idle) & is_rx_writing_go_CSR;

    assign xcel_all_A_req_sent = (xcel_state_r == xcel_load_A_send)
                               & xcel_master_sent & (reg_id_r == last_A_id);

    assign xcel_all_A_resp_back = (xcel_state_r == xcel_load_A_wait)
                                & (tx_credits_lo == max_out_credits_p);

    assign xcel_all_B_req_sent = (xcel_state_r == xcel_load_B_send)
                               & xcel_master_sent & (reg_id_r == last_B_id);

    assign xcel_all_B_resp_back = (xcel_state_r == xcel_load_B_wait)
                                & (tx_credits_lo == max_out_credits_p);

    assign xcel_read_a_finished = (xcel_state_r == xcel_read_a) & xcel_mem_port_go;

    assign xcel_read_b_finished = (xcel_state_r == xcel_read_b) & xcel_mem_port_go;

    assign xcel_wb_sum_finished = (xcel_state_r == xcel_wb_sum) & xcel_mem_port_go;

    assign xcel_last_wb = (xcel_state_r == xcel_wb_sum) & (wb_reg_id_r == last_A_id);

    assign xcel_signal_sent = (xcel_state_r == xcel_signal) & xcel_master_sent;

    assign xcel_signal_acked = (xcel_state_r == xcel_wait) & tx_returned_v_lo;

    // load_a/b_reg_id_r

    always_ff@(posedge clk_i) begin
      if( reset_i )
        load_a_reg_id_r <= 'b0;
      else if( (xcel_state_r == xcel_load_A_send) & xcel_master_sent )
        load_a_reg_id_r <= load_a_reg_id_r + 1'b1;
    end

    always_ff@(posedge clk_i) begin
      if( reset_i )
        load_b_reg_id_r <= 'b0;
      else if( (xcel_state_r == xcel_load_B_send) & xcel_master_sent )
        load_b_reg_id_r <= load_b_reg_id_r + 1'b1;
    end

    // Value a and b
    // the FSM will transit to wb_sum state when the request for B
    // succeeds. this means in wb_sum state we can combinationally
    // use the data from the SRAM for B.

    always_ff@(posedge clk_i) begin
      if( reset_i ) read_a_en_r <= 'b0;
      else          read_a_en_r <= xcel_read_a_finished;
    end

    always_ff@(posedge clk_i) begin
      if( reset_i )          value_a_r <= 'b0;
      else if( read_a_en_r ) value_a_r <= mem_data_lo[mem_xcel_port_lp];
    end

    assign value_b = mem_data_lo[mem_xcel_port_lp];

    // read_a/b_reg_id_r, wb_sum_reg_id_r

    always_ff@(posedge clk_i) begin
      if( reset_i )
        read_a_reg_id_r <= 'b0;
      else if( (xcel_state_r == xcel_read_a) & xcel_mem_port_go )
        read_a_reg_id_r <= read_a_reg_id_r + 1'b1;
    end 

    always_ff@(posedge clk_i) begin
      if( reset_i )
        read_b_reg_id_r <= 'b0;
      else if( (xcel_state_r == xcel_read_b) & xcel_mem_port_go )
        read_b_reg_id_r <= read_b_reg_id_r + 1'b1;
    end 

    always_ff@(posedge clk_i) begin
      if( reset_i )
        wb_reg_id_r <= 'b0;
      else if( (xcel_state_r == xcel_wb_sum) & xcel_mem_port_go )
        wb_reg_id_r <= wb_reg_id_r + 1'b1;
    end 

    //--------------------------------------------------------------
    // Connect local memory
    //--------------------------------------------------------------

    // the slave request from the processor
    assign mem_v_li   [mem_proc_port_lp] = rx_is_local_mem_lo & rx_v_lo;
    assign mem_we_li  [mem_proc_port_lp] = rx_we_lo;
    assign mem_data_li[mem_proc_port_lp] = rx_wdata_lo;
    assign mem_addr_li[mem_proc_port_lp] = rx_addr_lo;
    assign mem_mask_li[mem_proc_port_lp] = rx_mask_lo;

    // multiplexing the xcel port between different FSM states

    always_comb begin
      case( xcel_state_r )
        // the master response (loaded data) to be written into local memory
        xcel_load_A_send, xcel_load_A_wait, xcel_load_B_send, xcel_load_B_wait: begin
          mem_v_li   [mem_xcel_port_lp] = tx_returned_v_lo;
          mem_we_li  [mem_xcel_port_lp] = 1'b1;
          mem_data_li[mem_xcel_port_lp] = tx_returned_data_lo;
          mem_addr_li[mem_xcel_port_lp] = xcel_base_addr + tx_returned_reg_id_lo;
          mem_mask_li[mem_xcel_port_lp] = {(data_width_p>>3){1'b1}};
        end
        // read A from scratchpad
        xcel_read_a: begin
          mem_v_li   [mem_xcel_port_lp] = 1'b1;
          mem_we_li  [mem_xcel_port_lp] = 1'b0;
          mem_data_li[mem_xcel_port_lp] = 'b0;
          mem_addr_li[mem_xcel_port_lp] = xcel_A_base_addr + read_a_reg_id_r;
          mem_mask_li[mem_xcel_port_lp] = {(data_width_p>>3){1'b1}};
        end
        // read B from scratchpad
        xcel_read_b: begin
          mem_v_li   [mem_xcel_port_lp] = 1'b1;
          mem_we_li  [mem_xcel_port_lp] = 1'b0;
          mem_data_li[mem_xcel_port_lp] = 'b0;
          mem_addr_li[mem_xcel_port_lp] = xcel_B_base_addr + read_b_reg_id_r;
          mem_mask_li[mem_xcel_port_lp] = {(data_width_p>>3){1'b1}};
        end
        // write sum into scratchpad; overwrite A
        xcel_wb_sum: begin
          mem_v_li   [mem_xcel_port_lp] = 1'b1;
          mem_we_li  [mem_xcel_port_lp] = 1'b1;
          mem_data_li[mem_xcel_port_lp] = value_a_r + value_b;
          mem_addr_li[mem_xcel_port_lp] = xcel_A_base_addr + wb_reg_id_r;
          mem_mask_li[mem_xcel_port_lp] = {(data_width_p>>3){1'b1}};
        end
        default: begin
          mem_v_li   [mem_xcel_port_lp] = 1'b0;
          mem_we_li  [mem_xcel_port_lp] = 1'b0;
          mem_data_li[mem_xcel_port_lp] = 'b0;
          mem_addr_li[mem_xcel_port_lp] = 'b0;
          mem_mask_li[mem_xcel_port_lp] = 'b0;
        end
      endcase
    end

    //--------------------------------------------------------------
    // Connect RX
    //--------------------------------------------------------------

    assign rx_yumi_li = rx_is_CSR_lo       ? rx_v_lo
                      : rx_is_local_mem_lo ? rx_v_lo & mem_yumi_lo[mem_proc_port_lp]
                      : 1'b0;

    assign rx_returning_v_li = CSR_returning_v_r | mem_v_lo[mem_proc_port_lp];

    assign rx_returning_data_li = CSR_returning_v_r ? CSR_returning_data_r
                                                    : mem_data_lo[mem_proc_port_lp];

    //--------------------------------------------------------------
    // Connect TX
    //--------------------------------------------------------------

    assign tx_v_li = (xcel_state_r == xcel_load_A_send) | (xcel_state_r == xcel_load_B_send)
                   | (xcel_state_r == xcel_signal);
    assign tx_fetching_li = (xcel_state_r == xcel_load_A_send) | (xcel_state_r == xcel_load_B_send);
    assign tx_addr_li = (xcel_state_r == xcel_load_A_send) ? (addr_a.epa32 >> 2) + load_a_reg_id_r
                      : (xcel_state_r == xcel_load_B_send) ? (addr_b.epa32 >> 2) + load_b_reg_id_r
                      : 'b0;
    assign tx_signal_addr_li = addr_sig.epa32 >> 2;
    assign tx_reg_id_li = reg_id_r;

endmodule
