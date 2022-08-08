`include "bsg_manycore_defines.vh"
`include "bsg_defines.v"
module vanilla_core_dmem_tracker
  import bsg_vanilla_pkg::*;
  import bsg_manycore_addr_pkg::*;
    #(`BSG_INV_PARAM(data_width_p)
    , `BSG_INV_PARAM(dmem_size_p)
    
    , `BSG_INV_PARAM(icache_entries_p)
    , `BSG_INV_PARAM(icache_tag_width_p)

    , `BSG_INV_PARAM(x_cord_width_p)
    , `BSG_INV_PARAM(y_cord_width_p)

    , `BSG_INV_PARAM(pod_x_cord_width_p)
    , `BSG_INV_PARAM(pod_y_cord_width_p)

    , parameter credit_counter_width_p=`BSG_WIDTH(32)

    // For network input FIFO credit counting
      // By default, 3 credits are needed, because the round trip to get the credit back takes three cycles.
      // ID->EXE->FIFO->CREDIT.

    , dmem_addr_width_lp=`BSG_SAFE_CLOG2(dmem_size_p)
    , icache_addr_width_lp=`BSG_SAFE_CLOG2(icache_entries_p)
    , pc_width_lp=(icache_tag_width_p+icache_addr_width_lp)
    , reg_addr_width_lp = RV32_reg_addr_width_gp
    , data_mask_width_lp=(data_width_p>>3)

    , parameter debug_p=0
    , localparam max_tile_group_x_cord_width_gp = 6
    , localparam max_tile_group_y_cord_width_gp = 5
  )
  (
   input clk_i
   , input reset_i

   // remote dmem accesses
   , input remote_dmem_v_i
   , input remote_dmem_w_i
   , input [dmem_addr_width_lp-1:0] remote_dmem_addr_i
   , input [data_mask_width_lp-1:0] remote_dmem_mask_i
   , input [data_width_p-1:0] remote_dmem_data_i
   , input [data_width_p-1:0] remote_dmem_data_o
   , input remote_dmem_yumi_o

   // dmem access
   , input dmem_v_li
   , input dmem_w_li
   , input [data_width_p-1:0] dmem_data_li
   , input [dmem_addr_width_lp-1:0] dmem_addr_li
   , input [data_mask_width_lp-1:0] dmem_mask_li
   , input [data_width_p-1:0] dmem_data_lo

   // global coordinates
   , input [x_cord_width_p-1:0] global_x_i
   , input [y_cord_width_p-1:0] global_y_i

   // pcs for different pipeline stages
   , input [data_width_p-1:0] if_pc
   , input [data_width_p-1:0] id_pc
   , input [data_width_p-1:0] exe_pc
   );

  ///////////////////
  // DPI Functions //
  ///////////////////
`define cfunc \
  import "DPI-C" context function
  typedef bit [31:0] pc_t;
  typedef bit [31:0] addr_t;
  
  chandle dmem_tracker;  
  `cfunc chandle vanilla_core_dmem_tracker_new();
  `cfunc void    vanilla_core_dmem_tracker_delete
    (chandle trckr);
  `cfunc void    vanilla_core_dmem_tracker_post_write
    (chandle trckr, pc_t pc, addr_t addr);  
  
  initial begin
    dmem_tracker = vanilla_core_dmem_tracker_new();    
  end
  final begin
    vanilla_core_dmem_tracker_delete(dmem_tracker);    
  end  
  
  logic [data_width_p-1:0] dmem_addr_byte;
  assign dmem_addr_byte = dmem_addr_li<<2;

  always @(negedge clk_i) begin
    if (~reset_i) begin
      if (dmem_v_li & dmem_w_li & ~remote_dmem_v_i) begin
        vanilla_core_dmem_tracker_post_write
          (dmem_tracker
           ,exe_pc
           ,dmem_addr_byte
           );        
      end      
    end    
  end  
  
  if (debug_p) begin
    always @(negedge clk_i) begin
      if (~reset_i) begin
        if (dmem_v_li & ~remote_dmem_v_i) begin
          $display("[DMEM]: (x=%3d,y=%3d) r=%b, w=%b, addr=%08h, pc=%h"
                   , global_x_i
                   , global_y_i
                   , remote_dmem_v_i
                   , dmem_w_li
                   , dmem_addr_byte
                   , exe_pc
                   );
        end        
      end 
    end
   end // if (debug_p)
  
  always @(posedge reset_i) begin
    $display("%m: vanilla_core_dmem_tracker");
  end
  
endmodule // vanilla_core_dmem_tracker

