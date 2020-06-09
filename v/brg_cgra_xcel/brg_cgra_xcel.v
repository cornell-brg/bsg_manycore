//====================================================================
// brg_cgra_xcel.v
// Author : Peitian Pan
// Date   : May 7, 2020
//====================================================================
// CGRA accelerator with HB endpoint interfaces.

module brg_cgra_xcel
  import bsg_manycore_pkg::*;
  #( 
     x_cord_width_p         = "inv"
    ,y_cord_width_p         = "inv"
    ,dmem_size_p            = "inv"
    ,data_width_p           = 32
    ,addr_width_p           = 32
    ,max_out_credits_p      = 200
    ,packet_width_lp                = `bsg_manycore_packet_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
    ,return_packet_width_lp         = `bsg_manycore_return_packet_width(x_cord_width_p,y_cord_width_p,data_width_p)
    ,bsg_manycore_link_sif_width_lp = `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
    ,debug_p                = 1
    /* Dummy parameter for compatability with socket*/
    ,hetero_type_p          = 1
    ,epa_byte_addr_width_p  = "inv"
  )
  (   input clk_i
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
      ,.out_ready_o           ( out_ready_lo         )
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

    //--------------------------------------------------------------
    // Interface CGRA accelerator to the endpoint
    //--------------------------------------------------------------

    HBEndpointCGRAXcel_2x2 hb_cgra_xcel (
      .clk                   ( clk_i                 ),
      .reset                 ( reset_i               ),

      .my_x_i                ( my_x_i                ),
      .my_y_i                ( my_y_i                ),

      .in_addr_i             ( in_addr_lo            ),
      .in_data_i             ( in_data_lo            ),
      .in_load_info_i        ( in_load_info_lo       ),
      .in_mask_i             ( in_mask_lo            ),
      .in_src_x_cord_i       ( in_src_x_cord_lo      ),
      .in_src_y_cord_i       ( in_src_y_cord_lo      ),
      .in_v_i                ( in_v_lo               ),
      .in_we_i               ( in_we_lo              ),
      .in_yumi_o             ( in_yumi_li            ),

      .returning_data_o      ( returning_data_li     ),
      .returning_v_o         ( returning_v_li        ),

      .out_credits_i         ( out_credits_lo        ),
      .out_packet_o          ( out_packet_li         ),
      .out_ready_i           ( out_ready_lo          ),
      .out_v_o               ( out_v_li              ),

      .returned_data_r_i     ( returned_data_lo      ),
      .returned_fifo_full_i  ( returned_fifo_full_lo ),
      .returned_pkt_type_r_i ( returned_pkt_type_lo  ),
      .returned_reg_id_r_i   ( returned_reg_id_lo    ),
      .returned_v_r_i        ( returned_v_lo         ),
      .returned_yumi_o       ( returned_yumi_li      )
    );

  // synopsys translate_off

  string ce0, ce1, ce2, ce3;
  string pe0, pe1, pe2, pe3;
  string me0, me1, me2, me3;

  function string get_ce_trace(logic mem_req_en, logic mem_resp_en, logic send_cfg_en);
    if(mem_req_en)
      get_ce_trace = "M ";
    else if(mem_resp_en)
      get_ce_trace = " >";
    else if(send_cfg_en)
      get_ce_trace = "C ";
    else
      get_ce_trace = "  ";
  endfunction

  function string get_pe_trace(logic [4:0] op, logic [31:0] a, logic [31:0] b, logic [31:0] out, logic operand_deq_en, logic result_send_en, logic recv_cfg_en, int num_ops);
    string op_string, a_string, b_string, out_string;
    if(recv_cfg_en)
      get_pe_trace = ">C";
    else begin
      if(op == 0)
        op_string = "->";
      else if(op == 2)
        op_string = "+ ";
      else
        op_string = "NA";
      if(operand_deq_en & result_send_en)
        get_pe_trace = op_string;
      else
        get_pe_trace = "  ";
    end
  endfunction

  function string get_me_trace(logic mem_req_en, logic mem_resp_en, logic recv_cfg_en, int is_resp_dir_reversed);
    if(recv_cfg_en) begin
      get_me_trace = ">C";
    end
    else if(mem_req_en & mem_resp_en) begin
      if(is_resp_dir_reversed)
        get_me_trace = "M<";
      else
        get_me_trace = "M>";
    end
    else if(mem_req_en)
      get_me_trace = "M ";
    else if(mem_resp_en) begin
      if(is_resp_dir_reversed)
        get_me_trace = " <";
      else
        get_me_trace = " >";
    end
    else
      get_me_trace = "  ";
  endfunction

  always_ff @ (negedge clk_i) begin
    if(hb_cgra_xcel.cgra_xcel.ctrl.state == 1) begin
      ce0 = get_ce_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_s.mem_master__req__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_s.mem_master__resp__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_s.cfg_send__en);
      ce1 = get_ce_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_c__0.mem_master__req__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_c__0.mem_master__resp__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_c__0.cfg_send__en);
      ce2 = get_ce_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_c__1.mem_master__req__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_c__1.mem_master__resp__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_c__1.cfg_send__en);
      ce3 = get_ce_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_n.mem_master__req__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_n.mem_master__resp__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.CE_n.cfg_send__en);
      me0 = get_me_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__0.mem_ifc__req__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__0.mem_ifc__resp__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__0.ctrl.self_cfg_deq,
                         0);
      me1 = get_me_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__1.mem_ifc__req__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__1.mem_ifc__resp__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__1.ctrl.self_cfg_deq,
                         0);
      me2 = get_me_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__2.mem_ifc__req__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__2.mem_ifc__resp__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__2.ctrl.self_cfg_deq,
                         1);
      me3 = get_me_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__3.mem_ifc__req__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__3.mem_ifc__resp__en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.ME_r__3.ctrl.self_cfg_deq,
                         1);
      pe0 = get_pe_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__0.ctrl.cfg.func,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__0.dpath.opd_a_mux.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__0.dpath.opd_b_mux.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__0.dpath.node.alu.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__0.ctrl.operands_deq_rdy & hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__0.ctrl.node_recv_rdy,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__0.ctrl.node_send_en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__0.ctrl.self_cfg_deq,
                         1);
      pe1 = get_pe_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__1.ctrl.cfg.func,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__1.dpath.opd_a_mux.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__1.dpath.opd_b_mux.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__1.dpath.node.alu.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__1.ctrl.operands_deq_rdy & hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__1.ctrl.node_recv_rdy,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__1.ctrl.node_send_en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__1.ctrl.self_cfg_deq,
                         1);
      pe2 = get_pe_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__2.ctrl.cfg.func,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__2.dpath.opd_a_mux.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__2.dpath.opd_b_mux.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__2.dpath.node.alu.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__2.ctrl.operands_deq_rdy & hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__2.ctrl.node_recv_rdy,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__2.ctrl.node_send_en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__2.ctrl.self_cfg_deq,
                         1);
      pe3 = get_pe_trace(hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__3.ctrl.cfg.func,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__3.dpath.opd_a_mux.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__3.dpath.opd_b_mux.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__3.dpath.node.alu.out,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__3.ctrl.operands_deq_rdy & hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__3.ctrl.node_recv_rdy,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__3.ctrl.node_send_en,
                         hb_cgra_xcel.cgra_xcel.dpath.cgra.dpath.PE_rc__3.ctrl.self_cfg_deq,
                         2);
      // Only dump out trace when CGRA has launched
      $display("[CGRA_XCEL] %s %s %s | %s %s %s | %s %s %s | %s %s %s",
               ce0, me0, me1, ce1, pe0, pe1, ce2, pe2, pe3, ce3, me2, me3);
    end
  end

  // synopsys translate_on

endmodule
