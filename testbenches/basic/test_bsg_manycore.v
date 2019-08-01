`include "bsg_manycore_packet.vh"

`define DMEM_SIZE       1024  //in words
`define ICACHE_ENTRIES  1024
`define DRAM_CH_SIZE    1024  //in words

`ifndef bsg_global_X
`error bsg_global_X must be defined; pass it in through the makefile
`endif

`ifndef bsg_global_Y
`error bsg_global_Y must be defined; pass it in through the makefile
`endif


parameter int bsg_hetero_type_vec_gp [0:`bsg_global_Y-1][0:`bsg_global_X-1]  = '{ `bsg_hetero_type_vec };

`define MAX_CYCLES 200000000



`ifdef ENABLE_TRACE
`endif  // TRACE

module test_bsg_manycore;

   import  bsg_noc_pkg   ::*; // {P=0, W, E, N, S}

   localparam debug_lp = 0;
   localparam max_cycles_lp   = `MAX_CYCLES;
   localparam tile_id_ptr_lp  = -1;
   localparam dmem_size_lp    = `DMEM_SIZE ;
   localparam icache_entries_num_lp  = `ICACHE_ENTRIES;
   localparam icache_tag_width_lp= 12;      // 16MB PC address
   localparam data_width_lp   = 32;
   localparam load_id_width_lp = 11;
   localparam epa_byte_addr_width_lp       = 18;
   localparam num_tiles_x_lp  = `bsg_global_X;
   localparam num_tiles_y_lp  = `bsg_global_Y;
   localparam extra_io_rows_lp= 1;
   localparam lg_node_x_lp    = `BSG_SAFE_CLOG2(num_tiles_x_lp);
   localparam lg_node_y_lp    = `BSG_SAFE_CLOG2(num_tiles_y_lp + extra_io_rows_lp);
   localparam addr_width_lp   = 32-2-1-lg_node_x_lp+1;
   localparam dram_ch_addr_width_lp   =  32-2-1-lg_node_x_lp; // 2MB;
   localparam packet_width_lp        = `bsg_manycore_packet_width       (addr_width_lp, data_width_lp, lg_node_x_lp, lg_node_y_lp, load_id_width_lp);
   localparam return_packet_width_lp = `bsg_manycore_return_packet_width(lg_node_x_lp, lg_node_y_lp, data_width_lp, load_id_width_lp);
   localparam cycle_time_lp   = 1100; //20;
   localparam trace_vscale_pipeline_lp=0;
   localparam trace_manycore_tile_lp=0;
   localparam trace_manycore_proc_lp=0;

   wire finish_lo;

   if (trace_manycore_tile_lp)
     bind bsg_manycore_tile  bsg_manycore_tile_trace #(.packet_width_lp(packet_width_lp)
                                                       ,.return_packet_width_lp(return_packet_width_lp)
                                                       ,.x_cord_width_p(x_cord_width_p)
                                                       ,.y_cord_width_p(y_cord_width_p)
                                                       ,.addr_width_p(addr_width_p)
                                                       ,.data_width_p(data_width_p)
                                                       ,.load_id_width_p(load_id_width_p)
                                                       ,.bsg_manycore_link_sif_width_lp(bsg_manycore_link_sif_width_lp)
                                                       ) bmtt
       (.clk_i
        ,.links_sif_i
        ,.links_sif_o
        ,.my_x_i
        ,.my_y_i
        ,.freeze(freeze)
        );

   if (trace_vscale_pipeline_lp)
     bind   vscale_pipeline bsg_manycore_vscale_pipeline_trace #(.x_cord_width_p(x_cord_width_p)
                                                    ,.y_cord_width_p(y_cord_width_p)
                                                    ) vscale_trace(clk
                                                                   ,PC_IF
                                                                   ,wr_reg_WB
                                                                   ,reg_to_wr_WB
                                                                   ,wb_data_WB
                                                                   ,stall_WB
                                                                   ,imem_wait
                                                                   ,dmem_wait
                                                                   ,dmem_en
                                                                   ,exception_code_WB
                                                                   ,imem_addr
                                                                   ,imem_rdata
                                                                   ,freeze
                                                                   ,my_x_i
                                                                   ,my_y_i
                                                                   );
   if (trace_manycore_proc_lp)
     bind bsg_manycore_proc bsg_manycore_proc_trace #(.mem_width_lp(mem_width_lp)
                                                      ,.data_width_p(data_width_p)
                                                      ,.addr_width_p(addr_width_p)
                                                      ,.load_id_width_p(load_id_width_p)
                                                      ,.x_cord_width_p(x_cord_width_p)
                                                      ,.y_cord_width_p(y_cord_width_p)
                                                      ,.packet_width_lp(packet_width_lp)
                                                      ,.return_packet_width_lp(return_packet_width_lp)
                                                      ,.bsg_manycore_link_sif_width_lp(bsg_manycore_link_sif_width_lp)
                                                      ) proc_trace
       (clk_i
        ,xbar_port_v_in
        ,xbar_port_addr_in
        ,xbar_port_data_in
        ,xbar_port_mask_in
        ,xbar_port_we_in
        ,xbar_port_yumi_out
        ,my_x_i
        ,my_y_i
        ,link_sif_i
        ,link_sif_o

        ,freeze_r
        ,cgni_v
        ,cgni_data
        );


   localparam num_nets_lp = 2;

  // clock and reset generation
  wire clk;
  wire reset;

  bsg_nonsynth_clock_gen #( .cycle_time_p(cycle_time_lp)
                          ) clock_gen
                          ( .o(clk)
                          );

  bsg_nonsynth_reset_gen #(  .num_clocks_p     (1)
                           , .reset_cycles_lo_p(1)
                           , .reset_cycles_hi_p(10)
                          )  reset_gen
                          (  .clk_i        (clk)
                           , .async_reset_o(reset)
                          );

  // The manycore has a 2-FF pipelined reset in 16nm, therefore we need
  // to add a 2 cycle latency to all other modules.
  logic reset_r, reset_rr;
  always_ff @(posedge clk)
    begin
      reset_r <= reset;
      reset_rr <= reset_r;
    end

  integer       stderr = 32'h80000002;

   `declare_bsg_manycore_link_sif_s(addr_width_lp, data_width_lp, lg_node_x_lp, lg_node_y_lp, load_id_width_lp);

   bsg_manycore_link_sif_s [S:N][num_tiles_x_lp-1:0]   ver_link_li, ver_link_lo;
   bsg_manycore_link_sif_s [E:W][num_tiles_y_lp-1:0]   hor_link_li, hor_link_lo;
   bsg_manycore_link_sif_s      [num_tiles_x_lp-1:0]   io_link_li,  io_link_lo;

  bsg_manycore #
    (
      .dmem_size_p       (dmem_size_lp         )
     ,.icache_entries_p  (icache_entries_num_lp)
     ,.icache_tag_width_p(icache_tag_width_lp)
     ,.data_width_p (data_width_lp)
     ,.addr_width_p (addr_width_lp)
     ,.load_id_width_p (load_id_width_lp)
     ,.epa_byte_addr_width_p (epa_byte_addr_width_lp)
     ,.dram_ch_addr_width_p( dram_ch_addr_width_lp )
     ,.dram_ch_start_col_p ( 1'b0                  )
     ,.num_tiles_x_p(num_tiles_x_lp)
     ,.num_tiles_y_p(num_tiles_y_lp)
     ,.extra_io_rows_p ( extra_io_rows_lp  )
     ,.hetero_type_vec_p( bsg_hetero_type_vec_gp )
     // currently west side is stubbed except for upper left tile
     //,.stub_w_p     ({{(num_tiles_y_lp-1){1'b1}}, 1'b0})
     //,.stub_e_p     ({num_tiles_y_lp{1'b1}})
     //,.stub_n_p     ({num_tiles_x_lp{1'b1}})
     //
     // try unstubbing all
     ,.stub_w_p     ({num_tiles_y_lp{1'b0}})
     ,.stub_e_p     ({num_tiles_y_lp{1'b0}})
     ,.stub_n_p     ({num_tiles_x_lp{1'b0}})


     // south side is unstubbed
     ,.stub_s_p     ({num_tiles_x_lp{1'b0}})
     ,.debug_p(debug_lp)
    ) UUT
      ( .clk_i   (clk)
        ,.reset_i (reset)

        ,.hor_link_sif_i(hor_link_li)
        ,.hor_link_sif_o(hor_link_lo)

        ,.ver_link_sif_i(ver_link_li)
        ,.ver_link_sif_o(ver_link_lo)

        ,.io_link_sif_i(io_link_li)
        ,.io_link_sif_o(io_link_lo)

        );

/////////////////////////////////////////////////////////////////////////////////
// Tie the unused I/O
   genvar                   i,j;
   for (i = 0; i < num_tiles_y_lp; i=i+1)
     begin: rof2

        bsg_manycore_link_sif_tieoff #(.addr_width_p     (addr_width_lp  )
                                       ,.data_width_p    (data_width_lp  )
                                       ,.load_id_width_p (load_id_width_lp)
                                       ,.x_cord_width_p  (lg_node_x_lp)
                                       ,.y_cord_width_p  (lg_node_y_lp)
                                       ) bmlst
        (.clk_i(clk)
         ,.reset_i(reset_rr)
         ,.link_sif_i(hor_link_lo[W][i])
         ,.link_sif_o(hor_link_li[W][i])
         );

        bsg_manycore_link_sif_tieoff #(.addr_width_p     (addr_width_lp  )
                                       ,.data_width_p    (data_width_lp  )
                                       ,.load_id_width_p (load_id_width_lp)
                                       ,.x_cord_width_p  (lg_node_x_lp   )
                                       ,.y_cord_width_p  (lg_node_y_lp   )
                                       ) bmlst2
        (.clk_i(clk)
         ,.reset_i(reset_rr)
         ,.link_sif_i(hor_link_lo[E][i])
         ,.link_sif_o(hor_link_li[E][i])
         );
     end


   for (i = 0; i < num_tiles_x_lp; i=i+1)
     begin: rof
        // tie off north side; which is inaccessible
        bsg_manycore_link_sif_tieoff #(.addr_width_p     (addr_width_lp)
                                       ,.data_width_p    (data_width_lp)
                                       ,.load_id_width_p (load_id_width_lp)
                                       ,.x_cord_width_p  (lg_node_x_lp)
                                       ,.y_cord_width_p  (lg_node_y_lp)
                                       ) bmlst3
        (.clk_i(clk)
         ,.reset_i(reset_rr)
         ,.link_sif_i(ver_link_lo[N][i])
         ,.link_sif_o(ver_link_li[N][i])
         );
     end

/////////////////////////////////////////////////////////////////////////////////
// instantiate the loader and moniter

   bsg_nonsynth_manycore_io_complex
     #( .icache_entries_num_p(icache_entries_num_lp)
        ,.addr_width_p(addr_width_lp)
        ,.load_id_width_p(load_id_width_lp)
        ,.epa_byte_addr_width_p(epa_byte_addr_width_lp)
        ,.dram_ch_addr_width_p( dram_ch_addr_width_lp)
        ,.data_width_p(data_width_lp)
        ,.extra_io_rows_p ( extra_io_rows_lp )
	,.max_cycles_p(max_cycles_lp)
        ,.num_tiles_x_p(num_tiles_x_lp)
        ,.num_tiles_y_p(num_tiles_y_lp)
	,.tile_id_ptr_p(tile_id_ptr_lp)
        ) io
   (.clk_i(clk)
    ,.reset_i(reset_rr)
    ,.ver_link_sif_i(ver_link_lo[S])
    ,.ver_link_sif_o(ver_link_li[S])
    ,.io_link_sif_i(io_link_lo)
    ,.io_link_sif_o(io_link_li)
    ,.finish_lo(finish_lo)
    ,.success_lo()
    ,.timeout_lo()
    );


/////////////////////////////////////////////////////////////////////////////////
// instantiate the  profiler
//`define  PERF_COUNT
`define  TOPLEVEL UUT

`ifdef PERF_COUNT
genvar x,y;
  for (x = 0; x < num_tiles_x_lp; x++) begin: prof_x
    for (y = 0; y < num_tiles_y_lp; y++) begin: prof_y

          //generate the unfreeze signal
          wire  freeze_sig =  `TOPLEVEL.y[y].x[x].tile.proc.h.z.freeze_o;
          logic freeze_r;
          always @(negedge clk) freeze_r  <=  freeze_sig;
          assign          unfreeze_action  =  freeze_r & (~freeze_sig );

          //assign the inputs to the profiler
          manycore_profiler_s trigger_s;
          assign trigger_s.reset_prof   = unfreeze_action   ;
          assign trigger_s.finish_prof  = finish_lo         ;

          assign trigger_s.dmem_stall   = `TOPLEVEL.y[y].x[x].tile.proc.h.z.vanilla_core.stall_mem;
          assign trigger_s.dx_stall     = `TOPLEVEL.y[y].x[x].tile.proc.h.z.vanilla_core.depend_stall;
          assign trigger_s.bt_stall     = `TOPLEVEL.y[y].x[x].tile.proc.h.z.vanilla_core.flush;
          assign trigger_s.in_fifo_full = ~`TOPLEVEL.y[y].x[x].tile.proc.h.z.endp.bme.link_sif_o_cast.fwd.ready_and_rev;
          assign trigger_s.out_fifo_full= ~`TOPLEVEL.y[y].x[x].tile.proc.h.z.endp.bme.link_sif_i_cast.fwd.ready_and_rev;
          assign trigger_s.credit_full  = `TOPLEVEL.y[y].x[x].tile.proc.h.z.endp.out_credits_o == 0 ;
          assign trigger_s.res_acq_stall= `TOPLEVEL.y[y].x[x].tile.proc.h.z.vanilla_core.stall_lrw ;

          //instantiate the profiler
          bsg_manycore_profiler prof_inst(
                .clk_i      ( clk        )
               ,.x_id_i     ( x          )
               ,.y_id_i     ( y          )
               ,.prof_s_i   ( trigger_s  )
          );

    end
  end
`endif

//Generate SAIF File on-the-fly: not including Freeze mode by Shady Agwa June 2019
  localparam xd = 0;
  localparam yd = 1;
  wire [50:0] brg_wb_d;
  reg [31:0] brg_ci_count = 0;
  reg [31:0] brg_wb_count = 0;
  reg [31:0] brg_clk_cycles = 0;
  // Counted number of each  committed instruction.
  reg [31:0] brg_lw_c = 0;
  reg [31:0] brg_nop_c = 0;
  reg [31:0] brg_sw_c = 0;
  reg [31:0] brg_others_c = 0;
  reg [31:0] brg_addi_c = 0;
  reg [31:0] brg_lui_c = 0;
  reg [31:0] brg_or_c = 0;
  reg [31:0] brg_slli_c = 0;
  reg [31:0] brg_add_c = 0;
  reg [31:0] brg_j_c = 0;
  reg [31:0] brg_bne_c = 0;
  reg [31:0] brg_beq_c = 0;
  reg [31:0] brg_li_c = 0;
  reg [31:0] brg_mv_c = 0;

  reg [31:0] brg_sub_c = 0;
  reg [31:0] brg_and_c = 0;
  reg [31:0] brg_xor_c = 0;
  reg [31:0] brg_srl_c = 0;
  reg [31:0] brg_sra_c = 0;
  reg [31:0] brg_sll_c = 0;
  reg [31:0] brg_slt_c = 0;
  reg [31:0] brg_sltu_c = 0;
  reg [31:0] brg_srli_c = 0;
  reg [31:0] brg_srai_c = 0;
  reg [31:0] brg_andi_c = 0;
  reg [31:0] brg_ori_c = 0;
  reg [31:0] brg_xori_c = 0;
  reg [31:0] brg_slti_c = 0;
  reg [31:0] brg_sltiu_c = 0;
  reg [31:0] brg_jal_c = 0;
  reg [31:0] brg_mul_c = 0;
  reg [31:0] brg_div_c = 0;

  integer f;
  wire freezo;
  wire brg_stats_en;
  wire brg_tiles_or;
  wire brg_tiles_stats_en;
  reg [50:0] brg_wb_d_r;
  wire brg_wb_tr;
  // finish execution signal goes high -> stop toggle tracing and report saif  file
  wire brg_finish;
  real brg_cpi;
  
  assign brg_finish = finish_lo;
  assign brg_cpi = $bitstoreal( brg_clk_cycles)/$bitstoreal((brg_ci_count-4));
  assign brg_stats_en = `TOPLEVEL.y[yd].x[xd].tile.proc.h.z.vanilla_core.stats_en; 

  assign brg_wb_d [50-:51] = `TOPLEVEL.y[yd].x[xd].tile.proc.h.z.vanilla_core.debug_wb[50-:51];
  
  assign freezo = `TOPLEVEL.y[yd].x[xd].tile.proc.h.z.freeze_o;
 
  // brg_stats_en goes high -> start toggle tracing 
  always @( posedge brg_stats_en)
  begin
    $set_gate_level_monitoring( "rtl_on" );
    $set_toggle_region( UUT );
    $toggle_start;
  end
  

  // finish execution signal goes high
  always @(brg_finish)
  begin
    if (brg_finish == 1)
      begin
        $display("\n******************************************\n");
        $display("\n BRG: | # Total Instructions:(%d) | # Committed Instructions:(%d) | \n", brg_wb_count-4, brg_ci_count-4);
        $display("\n BRG: | # clock Cycles within Stats_en Active:(%d) | CPI: (%f) \n", brg_clk_cycles, brg_cpi);
//        $display("\n # nop:\t%d \n # lw:\t%d \n # sw:\t%d \n # addi:\t%d \n # lui:\t%d \n # or:\t%d \n # slli:\t%d \n # add:\t%d \n # j:\t%d \n # bne:\t%d \n # beq:\t%d \n # li:\t%d \n # mv:\t%d \n # Others:\t%d \n", brg_nop_c-3, brg_lw_c, brg_sw_c, brg_addi_c, brg_lui_c, brg_or_c,brg_slli_c, brg_add_c, brg_j_c, brg_bne_c, brg_beq_c, brg_li_c, brg_mv_c, brg_others_c-1);
        $display("\n******************************************\n");
        // Write stats in brg_stats.json file.
	$fwrite(f,"{\n");
        $fwrite(f,"\"total-instructions\": %d,\n", brg_wb_count-4);
        $fwrite(f,"\"committed-instructions\": %d,\n", brg_ci_count-4);
        $fwrite(f,"\"n-cycles\": %d,\n",  brg_clk_cycles);
        $fwrite(f,"\"cpi\": %f,\n",  brg_cpi);

        $fwrite(f,"\"nop\": %d,\n", brg_nop_c-3);
        $fwrite(f,"\"lw\": %d,\n", brg_lw_c);
        $fwrite(f,"\"sw\": %d,\n", brg_sw_c);
        $fwrite(f,"\"addi\": %d,\n", brg_addi_c);
        $fwrite(f,"\"lui\": %d,\n", brg_lui_c);
        $fwrite(f,"\"or\": %d,\n", brg_or_c);
        $fwrite(f,"\"slli\": %d,\n", brg_slli_c);
        $fwrite(f,"\"add\": %d,\n", brg_add_c);
        $fwrite(f,"\"j\": %d,\n", brg_j_c);
        $fwrite(f,"\"bne\": %d,\n", brg_bne_c);
        $fwrite(f,"\"beq\": %d,\n", brg_beq_c);
        $fwrite(f,"\"li\": %d,\n", brg_li_c);
        $fwrite(f,"\"mv\": %d,\n", brg_mv_c);

        $fwrite(f,"\"sub\": %d,\n", brg_sub_c);
        $fwrite(f,"\"and\": %d,\n", brg_and_c);
        $fwrite(f,"\"xor\": %d,\n", brg_xor_c);
        $fwrite(f,"\"srl\": %d,\n", brg_srl_c);
        $fwrite(f,"\"sra\": %d,\n", brg_sra_c);
        $fwrite(f,"\"sll\": %d,\n", brg_sll_c);
        $fwrite(f,"\"slt\": %d,\n", brg_slt_c);
        $fwrite(f,"\"sltu\": %d,\n", brg_sltu_c);
        $fwrite(f,"\"srli\": %d,\n", brg_srli_c);
        $fwrite(f,"\"srai\": %d,\n", brg_srai_c);
        $fwrite(f,"\"andi\": %d,\n", brg_andi_c);
        $fwrite(f,"\"ori\": %d,\n", brg_ori_c);
        $fwrite(f,"\"xori\": %d,\n", brg_xori_c);
        $fwrite(f,"\"slti\": %d,\n", brg_slti_c);
        $fwrite(f,"\"sltiu\": %d,\n", brg_sltiu_c);
        $fwrite(f,"\"jal\": %d,\n", brg_jal_c);
        $fwrite(f,"\"mul\": %d,\n", brg_mul_c);
        $fwrite(f,"\"div\": %d,\n", brg_div_c);

        $fwrite(f,"\"others\": %d\n", brg_others_c-1);


        $fwrite(f,"}\n");

        $fclose(f); 
      end
  end
  // brg_stats_en goes low -> Stop toggle tracing
  always @(negedge brg_stats_en)
  begin
    $toggle_stop;
    $toggle_report( "run.saif", 1.0e-9, UUT );
  end
  // save the previous valie of brg_wb_d if no freeze mode
  always @(posedge clk)
  begin
    if (reset)
    begin
      brg_wb_d_r <= 0;
    end
    else if (freezo == 0)
    begin
      brg_wb_d_r <= brg_wb_d;
    end 
  end
  // check if there is a new instruction 
  assign brg_wb_tr = (brg_stats_en == 1) && (brg_wb_d_r != brg_wb_d) && (brg_wb_d != 1); // they are not the same == new commit inst.
  // Count the number of Committed instructions 
  always @ (negedge clk)
  begin
    if ((brg_wb_tr == 1) && (brg_wb_d[0] == 0))
    begin
      brg_ci_count = brg_ci_count + 1;
    end
  end

  // Count the number of wb instructions
  always @ (negedge clk)
  begin
    if (brg_wb_tr == 1)
    begin
      brg_wb_count = brg_wb_count + 1;
    end
  end

  // Count the number of lw instructions
  always @ (negedge clk)
  begin
    if ((brg_wb_tr == 1) && (brg_wb_d[0] == 0))
    begin
      // LW count
      if (brg_wb_d[34:3] ==? `RV32_LW)
      begin
        brg_lw_c = brg_lw_c + 1;
      end
      // SW count
      else if (brg_wb_d[34:3] ==? `RV32_SW)
      begin
        brg_sw_c = brg_sw_c + 1;
      end
      // NOP count
      else if (brg_wb_d[34:3] == 32'h13) 
       begin
         brg_nop_c = brg_nop_c + 1;
       end
       // ADDI count
       else if (brg_wb_d[34:3] ==? `RV32_ADDI)
       begin
         // ADDI R[i],R[i], IMM = LI R[i], IMM
         if (brg_wb_d[22:15] == 8'h00)
         begin
            brg_li_c = brg_li_c + 1;
         end
         // ADDI R[i],R[j], 0 = MV R[i], R[j]
         else if(brg_wb_d[34:23] == 12'h000)
         begin
            brg_mv_c = brg_mv_c + 1;
         end
         // ADDI Normal Instruction
         else
         begin
           brg_addi_c = brg_addi_c + 1;
         end
       end
      // LUI count
      else if (brg_wb_d[34:3] ==? `RV32_LUI)
      begin
        brg_lui_c = brg_lui_c + 1;
      end
      // OR  count
      else if (brg_wb_d[34:3] ==? `RV32_OR)
      begin
        brg_or_c = brg_or_c + 1;
      end
      // SLLI  count
      else if (brg_wb_d[34:3] ==? `RV32_SLLI)
      begin
        brg_slli_c = brg_slli_c + 1;
      end
      // ADD count
      else if (brg_wb_d[34:3] ==? `RV32_ADD)
      begin
        brg_add_c = brg_add_c + 1;
      end
      // BNE  count
      else if (brg_wb_d[34:3] ==? `RV32_BNE)
      begin
        brg_bne_c = brg_bne_c + 1;
      end
      // BEQ count
      else if (brg_wb_d[34:3] ==? `RV32_BEQ)
      begin
        brg_beq_c = brg_beq_c + 1;
      end
      //J  count
      else if (brg_wb_d[14:3] == 12'h06f)
      begin
        brg_j_c = brg_j_c + 1;
      end
      // SUB count
      else if (brg_wb_d[34:3] ==? `RV32_SUB)
      begin
        brg_sub_c = brg_sub_c + 1;
      end
      // AND count
      else if (brg_wb_d[34:3] ==? `RV32_AND)
      begin
        brg_and_c = brg_and_c + 1;
      end
      // XOR count
      else if (brg_wb_d[34:3] ==? `RV32_XOR)
      begin
        brg_xor_c = brg_xor_c + 1;
      end
      // SRL count
      else if (brg_wb_d[34:3] ==? `RV32_SRL)
      begin
        brg_srl_c = brg_srl_c + 1;
      end
      // SRA count
      else if (brg_wb_d[34:3] ==? `RV32_SRA)
      begin
        brg_sra_c = brg_sra_c + 1;
      end
      // SLL count
      else if (brg_wb_d[34:3] ==? `RV32_SLL)
      begin
        brg_sll_c = brg_sll_c + 1;
      end
      // SLT count
      else if (brg_wb_d[34:3] ==? `RV32_SLT)
      begin
        brg_slt_c = brg_slt_c + 1;
      end
      // SLTU count
      else if (brg_wb_d[34:3] ==? `RV32_SLTU)
      begin
        brg_sltu_c = brg_sltu_c + 1;
      end
      // SRLI count
      else if (brg_wb_d[34:3] ==? `RV32_SRLI)
      begin
        brg_srli_c = brg_srli_c + 1;
      end
      // SRAI count
      else if (brg_wb_d[34:3] ==? `RV32_SRAI)
      begin
        brg_srai_c = brg_srai_c + 1;
      end
      // ANDI count
      else if (brg_wb_d[34:3] ==? `RV32_ANDI)
      begin
        brg_andi_c = brg_andi_c + 1;
      end
      // ORI count
      else if (brg_wb_d[34:3] ==? `RV32_ORI)
      begin
        brg_ori_c = brg_ori_c + 1;
      end
      // XORI count
      else if (brg_wb_d[34:3] ==? `RV32_XORI)
      begin
        brg_xori_c = brg_xori_c + 1;
      end
      // SLTI count
      else if (brg_wb_d[34:3] ==? `RV32_SLTI)
      begin
        brg_slti_c = brg_slti_c + 1;
      end
      // SLTIU count
      else if (brg_wb_d[34:3] ==? `RV32_SLTIU)
      begin
        brg_sltiu_c = brg_sltiu_c + 1;
      end
      // JAL count
      else if (brg_wb_d[34:3] ==? `RV32_JAL)
      begin
        brg_jal_c = brg_jal_c + 1;
      end
      // MUL count
      else if (brg_wb_d[34:3] ==? `RV32_MUL)
      begin
        brg_mul_c = brg_mul_c + 1;
      end
      // DIV count
      else if (brg_wb_d[34:3] ==? `RV32_DIV)
      begin
        brg_div_c = brg_div_c + 1;
      end
       // Other instructions count
       else 
       begin
         brg_others_c = brg_others_c + 1;
       end
    end 
  end

  // Count the number of clock cycles for the mbmark 
  always @ (negedge clk )
  begin
    if (brg_stats_en)
    begin
      brg_clk_cycles = brg_clk_cycles + 1;
    end
  end

  initial begin
    // Create json format file for the stats
    f = $fopen("brg_stats.json","w");
  end

endmodule
