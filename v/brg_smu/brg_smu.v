//========================================================================
// brg_smu.v
//
// Author : Peitian Pan
// Date   : Nov 23, 2020
//========================================================================

module brg_smu
  import bsg_manycore_pkg::*
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
  );

    //--------------------------------------------------------------
    // Endpoint standard
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
    logic                                  out_v_li              ;
    logic[packet_width_lp-1:0]             out_packet_li         ;
    logic                                  out_credit_or_ready_lo;
    logic[$clog2(max_out_credits_p+1)-1:0] out_credits_lo        ;

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

      // master request
      ,.out_v_i               ( out_v_li             )
      ,.out_packet_i          ( out_packet_li        )
      ,.out_credit_or_ready_o ( out_ready_lo         )
      ,.out_credits_o         ( out_credits_lo       )

      // master reponse
      ,.returned_data_r_o     ( returned_data_lo      )
      ,.returned_reg_id_r_o   ( returned_reg_id_lo    )
      ,.returned_v_r_o        ( returned_v_lo         )
      ,.returned_pkt_type_r_o ( returned_pkt_type_lo  )
      ,.returned_yumi_i       ( returned_yumi_li      )
      ,.returned_fifo_full_o  ( returned_fifo_full_lo )
    );

    //--------------------------------------------------------------
    // HBEndpointSMU
    //--------------------------------------------------------------

    `declare_bsg_manycore_packet_s(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p);

    bsg_manycore_packet_s    hb_smu_out_pkt;
    bsg_manycore_packet_s    out_packet_struct;
    logic [data_width_p-1:0] out_pkt_eva_lo;

    HBEndpointSMU_11x18 hb_smu (
       .clk                   ( clk_i                  )
      ,.reset                 ( reset_i                )

      ,.my_x_i                ( my_x_i                 )
      ,.my_y_i                ( my_y_i                 )

      ,.in_v_i                ( in_v_lo                )
      ,.in_data_i             ( in_data_lo             )
      ,.in_mask_i             ( in_mask_lo             )
      ,.in_addr_i             ( in_addr_lo             )
      ,.in_we_i               ( in_we_lo               )
      ,.in_load_info_i        ( in_load_info_lo        )
      ,.in_src_x_cord_i       ( in_src_x_cord_lo       )
      ,.in_src_y_cord_i       ( in_src_y_cord_lo       )
      ,.in_yumi_o             ( in_yumi_li             )

      ,.returning_data_o      ( returning_data_li      )
      ,.returning_v_o         ( returning_v_li         )

      ,.out_v_o               ( out_v_li               )
      ,.out_packet_o          ( hb_smu_out_pkt         )
      ,.out_credit_or_ready_i ( out_credit_or_ready_lo )
      ,.out_credits_i         ( out_credits_lo         )
      ,.out_pkt_eva_o         ( out_pkt_eva_lo         )

      ,.returned_data_r_i     ( returned_data_lo       )
      ,.returned_reg_id_r_i   ( returned_reg_id_lo     )
      ,.returned_v_r_i        ( returned_v_lo          )
      ,.returned_pkt_type_r_i ( returned_pkt_type_lo   )
      ,.returned_yumi_o       ( returned_yumi_li       )
      ,.returned_fifo_full_i  ( returned_fifo_full_lo  )
    );

    assign out_packet_li = out_packet_struct;

    //--------------------------------------------------------------
    // Address translation
    //--------------------------------------------------------------
    // For now assume dram is enabled by default. Also users of the
    // SMU should only use global addresses which means we don't
    // need to supply a tile group origin.

    logic [x_cord_width_p-1:0] x_cord_lo;
    logic [y_cord_width_p-1:0] y_cord_lo;
    logic [addr_width_p-1:0]   epa_lo;
    logic is_invalid_addr_lo;

    bsg_manycore_eva_to_npa
    #(
       .data_width_p                 ( data_width_p                 )
      ,.addr_width_p                 ( addr_width_p                 )
      ,.x_cord_width_p               ( x_cord_width_p               )
      ,.y_cord_width_p               ( y_cord_width_p               )
      ,.num_tiles_x_p                ( num_tiles_x_p                )
      ,.num_tiles_y_p                ( num_tiles_y_p                )
      ,.vcache_block_size_in_words_p ( vcache_block_size_in_words_p )
      ,.vcache_size_p                ( vcache_size_p                )
      ,.vcache_sets_p                ( vcache_sets_p                )
      ,.mc_composition_p             ( mc_composition_p             )
    )
    (
       .eva_i             ( out_pkt_eva_lo     )
      ,.tgo_x_i           ( 0                  )
      ,.tgo_y_i           ( 0                  )
      ,.dram_enable_i     ( 1                  )
      ,.x_cord_o          ( x_cord_lo          )
      ,.y_cord_o          ( y_cord_lo          )
      ,.epa_o             ( epa_lo             )
      ,.is_invalid_addr_o ( is_invalid_addr_lo )
    );

    // Packet builder

    assign out_packet_struct.addr       = epa_lo;
    assign out_packet_struct.op         = hb_smu_out_pkt.op;
    assign out_packet_struct.op_ex      = hb_smu_out_pkt.op_ex;
    assign out_packet_struct.reg_id     = hb_smu_out_pkt.reg_id;
    assign out_packet_struct.payload    = hb_smu_out_pkt.payload;
    assign out_packet_struct.src_y_cord = hb_smu_out_pkt.src_y_cord;
    assign out_packet_struct.src_x_cord = hb_smu_out_pkt.src_x_cord;
    assign out_packet_struct.y_cord     = y_cord_lo;
    assign out_packet_struct.y_cord     = x_cord_lo;

    //--------------------------------------------------------------
    // Diagnostic and debug info
    //--------------------------------------------------------------

    // synopsys translate_off

    always_ff @ (negedge clk_i) begin
      if (out_v_li & out_credit_or_ready_lo) begin
        assert(!is_invalid_addr_lo)
          else $error("[ERROR][HB-SMU] Invalid EVA address generated by SMU!");
      end

      if (returned_v_r_i) begin
        assert(returned_pkt_type_r_i != e_return_credit)
          else $error("[ERROR][HB-SMU] Credit packet should not be given to SMU!");
      end
    end

    // synopsys translate_on

endmodule
