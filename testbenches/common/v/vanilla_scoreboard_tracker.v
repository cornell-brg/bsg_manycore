`include "bsg_manycore_defines.vh"
`include "bsg_vanilla_defines.vh"
module vanilla_scoreboard_tracker
  import bsg_manycore_pkg::*;
  import bsg_vanilla_pkg::*;
  import vanilla_scoreboard_tracker_pkg::*;
  #(parameter `BSG_INV_PARAM(data_width_p)
    ,parameter reg_addr_width_lp=RV32_reg_addr_width_gp
    ,parameter `BSG_INV_PARAM(x_cord_width_p)
    ,parameter `BSG_INV_PARAM(y_cord_width_p)
    ,parameter `BSG_INV_PARAM(origin_x_cord_p)
    ,parameter `BSG_INV_PARAM(origin_y_cord_p)
    )
  (input clk_i
   ,input reset_i

   ,input flush
   ,input stall_all
   ,input stall_id

   ,input [data_width_p-1:0] rs1_val_to_exe
   ,input [data_width_p-1:0] mem_addr_op2

   ,input int_sb_clear
   ,input float_sb_clear
   ,input [reg_addr_width_lp-1:0] int_sb_clear_id
   ,input [reg_addr_width_lp-1:0] float_sb_clear_id

   ,input id_signals_s id_r
   ,input exe_signals_s exe_r
   ,input fp_exe_ctrl_signals_s fp_exe_ctrl_r

   ,input [x_cord_width_p-1:0] global_x_i
   ,input [y_cord_width_p-1:0] global_y_i

   ,output vanilla_isb_info_s [RV32_reg_els_gp-1:0] int_sb_o
   ,output vanilla_fsb_info_s [RV32_reg_els_gp-1:0] float_sb_o
   );

  // remote/local scoreboard tracking
  //
  // int_sb[3]: idiv
  // int_sb[2]: remote dram load
  // int_sb[1]: remote global load
  // int_sb[0]: remote group load
  //
  // float_sb[3]: fdiv / fsqrt
  // float_sb[2]: remote dram load
  // float_sb[1]: remote global load
  // float_sb[0]: remote group load

  vanilla_isb_info_s [RV32_reg_els_gp-1:0] int_sb_r;
  vanilla_fsb_info_s [RV32_reg_els_gp-1:0] float_sb_r;

  wire [data_width_p-1:0] id_mem_addr = rs1_val_to_exe + `BSG_SIGN_EXTEND(mem_addr_op2,data_width_p);

  // LC: check if ld_group is actualy made to local DMEM
  bsg_manycore_tile_group_addr_s tile_group_addr;
  assign tile_group_addr = id_mem_addr;
  wire is_my_x_addr = tile_group_addr.x_cord == (global_x_i - origin_x_cord_p);
  wire is_my_y_addr = tile_group_addr.y_cord == (global_y_i - origin_y_cord_p);
  wire is_my_addr   = is_my_x_addr & is_my_y_addr;
  wire is_true_remote_group_addr = (tile_group_addr.remote == 3'b001) & (~is_my_addr | id_r.decode.is_amo_op);

  wire is_overflow_dmem_addr = (tile_group_addr.remote == 3'b001) & is_my_addr & (tile_group_addr.addr inside {[16'h0080:16'hFFFF]});

  wire remote_ld_dram_in_id = (id_r.decode.is_load_op & id_r.decode.write_rd) & id_mem_addr[data_width_p-1];
  wire remote_amo_dram_in_id = id_r.decode.is_amo_op & id_mem_addr[data_width_p-1];
  wire remote_ld_global_in_id = ((id_r.decode.is_load_op & id_r.decode.write_rd) | id_r.decode.is_amo_op) & (id_mem_addr[data_width_p-1-:2] == 2'b01);
  wire remote_ld_group_in_id = (id_r.decode.is_load_op & id_r.decode.write_rd) & is_true_remote_group_addr;
  wire remote_amo_group_in_id = id_r.decode.is_amo_op & (id_mem_addr[data_width_p-1-:3] == 3'b001);
  wire dmem_overflow_ld_in_id = (id_r.decode.is_load_op & id_r.decode.write_rd) & is_overflow_dmem_addr;

  wire remote_flw_dram_in_id = (id_r.decode.is_load_op & id_r.decode.write_frd) & id_mem_addr[data_width_p-1];
  wire remote_flw_global_in_id = (id_r.decode.is_load_op & id_r.decode.write_frd) & (id_mem_addr[data_width_p-1-:2] == 2'b01);
  wire remote_flw_group_in_id = (id_r.decode.is_load_op & id_r.decode.write_frd) & ((id_mem_addr[data_width_p-1-:3] == 3'b001) & ~is_my_addr);
  wire dmem_overflow_fld_in_id = (id_r.decode.is_load_op & id_r.decode.write_frd) & is_overflow_dmem_addr;

  wire [reg_addr_width_lp-1:0] id_rd = id_r.instruction.rd;


  always_ff @ (posedge clk_i) begin
    if (reset_i) begin
      int_sb_r <= '0;
      float_sb_r <= '0;
    end
    else begin

      // int sb
      for (integer i = 0; i < RV32_reg_els_gp; i++) begin
        // idiv
        if (~stall_id & ~stall_all & ~flush & id_r.decode.is_idiv_op & (id_rd == i)) begin
          int_sb_r[i].idiv <= 1'b1;
        end
        else if (int_sb_clear & (int_sb_clear_id == i)) begin
          int_sb_r[i].idiv <= 1'b0;
        end
        // remote ld dram
        if (~stall_id & ~stall_all & ~flush & remote_ld_dram_in_id & (id_rd == i)) begin
          int_sb_r[i].remote_dram_load <= 1'b1;
        end
        else if (int_sb_clear & (int_sb_clear_id == i)) begin
          int_sb_r[i].remote_dram_load <= 1'b0;
        end
        // remote amo dram
        if (~stall_id & ~stall_all & ~flush & remote_amo_dram_in_id & (id_rd == i)) begin
          int_sb_r[i].remote_dram_amo <= 1'b1;
        end
        else if (int_sb_clear & (int_sb_clear_id == i)) begin
          int_sb_r[i].remote_dram_amo <= 1'b0;
        end
        // dmem overflow to dram
        if (~stall_id & ~stall_all & ~flush & dmem_overflow_ld_in_id & (id_rd == i)) begin
          int_sb_r[i].remote_dmem_overflow_load <= 1'b1;
        end
        else if (int_sb_clear & (int_sb_clear_id == i)) begin
          int_sb_r[i].remote_dmem_overflow_load <= 1'b0;
        end
        // remote ld global
        if (~stall_id & ~stall_all & ~flush & remote_ld_global_in_id & (id_rd == i)) begin
          int_sb_r[i].remote_global_load <= 1'b1;
        end
        else if (int_sb_clear & (int_sb_clear_id == i)) begin
          int_sb_r[i].remote_global_load <= 1'b0;
        end
        // remote ld group
        if (~stall_id & ~stall_all & ~flush & remote_ld_group_in_id & (id_rd == i)) begin
          int_sb_r[i].remote_group_load <= 1'b1;
        end
        else if (int_sb_clear & (int_sb_clear_id == i)) begin
          int_sb_r[i].remote_group_load <= 1'b0;
        end
        // remote amo group
        if (~stall_id & ~stall_all & ~flush & remote_amo_group_in_id & (id_rd == i)) begin
          int_sb_r[i].remote_group_amo <= 1'b1;
        end
        else if (int_sb_clear & (int_sb_clear_id == i)) begin
          int_sb_r[i].remote_group_amo <= 1'b0;
        end
      end // for (integer i = 0; i < RV32_reg_els_gp; i++)

      // float sb
      for (integer i = 0; i < RV32_reg_els_gp; i++) begin
        // fdiv, fsqrt
        if (~stall_id & ~stall_all & ~flush & (id_r.decode.is_fp_op & (id_r.fp_decode.is_fdiv_op | id_r.fp_decode.is_fsqrt_op)) & (id_rd == i)) begin
          float_sb_r[i].fdiv_fsqrt <= 1'b1;
        end
        else if (float_sb_clear & (float_sb_clear_id == i)) begin
          float_sb_r[i].fdiv_fsqrt <= 1'b0;
        end
        // remote flw dram
        if (~stall_id & ~stall_all & ~flush & remote_flw_dram_in_id & (id_rd == i)) begin
          float_sb_r[i].remote_dram_load <= 1'b1;
        end
        else if (float_sb_clear & (float_sb_clear_id == i)) begin
          float_sb_r[i].remote_dram_load <= 1'b0;
        end
        // remote flw global
        if (~stall_id & ~stall_all & ~flush & remote_flw_global_in_id & (id_rd == i)) begin
          float_sb_r[i].remote_global_load <= 1'b1;
        end
        else if (float_sb_clear & (float_sb_clear_id == i)) begin
          float_sb_r[i].remote_global_load <= 1'b0;
        end
        // remote flw group
        if (~stall_id & ~stall_all & ~flush & remote_flw_group_in_id & (id_rd == i)) begin
          float_sb_r[i].remote_group_load <= 1'b1;
        end
        else if (float_sb_clear & (float_sb_clear_id == i)) begin
          float_sb_r[i].remote_group_load <= 1'b0;
        end
        //flw dmem overflow to dream
        if (~stall_id & ~stall_all & ~flush & dmem_overflow_fld_in_id & (id_rd == i)) begin
          float_sb_r[i].remote_dmem_overflow_load <= 1'b1;
        end
        else if (float_sb_clear & (float_sb_clear_id == i)) begin
          float_sb_r[i].remote_dmem_overflow_load <= 1'b0;
        end
      end // for (integer i = 0; i < RV32_reg_els_gp; i++)
    end // else: !if(reset_i)
  end // always_ff @ (posedge clk_i)

  assign int_sb_o = int_sb_r;
  assign float_sb_o = float_sb_r;
endmodule // vanilla_scoreboard_tracker

`BSG_ABSTRACT_MODULE(vanilla_scoreboard_tracker)
