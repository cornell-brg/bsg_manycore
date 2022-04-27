/**
 *    bsg_manycore_eva_to_npa.v
 *
 *    This modules converts Endpoint Virtual Address (EVA) (32-bit) to Network Physical Address (NPA) (x,y-cord + EPA).
 *    Some EVA space maps to endpoint-specific address space. For example, vanilla core maps some portion of its EVA to its own local DMEM.
 *    It is very conceivable that different kinds of accelerators will map the portion of EVA differently to its local resources.
 *    This converter handles the mapping of EVA that is universal to all endpoints connected to the mesh network.
 *      1) DRAM
 *      2) Global
 *      3) Tile-Group
 *
 *    Modifying this mapping requires the same change in the following files.
 *    - Cuda-lite
 *    https://github.com/bespoke-silicon-group/bsg_replicant/blob/master/libraries/bsg_manycore_eva.cpp
 *    - SPMD testbench
 *    https://github.com/bespoke-silicon-group/bsg_manycore/blob/master/software/py/nbf.py
 */

`include "bsg_defines.v"

module bsg_manycore_eva_to_npa
  import bsg_manycore_pkg::*;
  #(`BSG_INV_PARAM(data_width_p) // 32
    , `BSG_INV_PARAM(addr_width_p)
    , `BSG_INV_PARAM(x_cord_width_p)
    , `BSG_INV_PARAM(y_cord_width_p)
    , `BSG_INV_PARAM(pod_x_cord_width_p)
    , `BSG_INV_PARAM(pod_y_cord_width_p)
 
    , `BSG_INV_PARAM(num_tiles_x_p)
    , `BSG_INV_PARAM(num_tiles_y_p)
    , localparam x_subcord_width_lp=`BSG_SAFE_CLOG2(num_tiles_x_p)
    , y_subcord_width_lp=`BSG_SAFE_CLOG2(num_tiles_y_p)

    , parameter `BSG_INV_PARAM(num_vcache_rows_p )
    , `BSG_INV_PARAM(vcache_block_size_in_words_p)  // block size in vcache
    , `BSG_INV_PARAM(vcache_size_p) // vcache capacity in words
    , `BSG_INV_PARAM(vcache_sets_p) // number of sets in vcache
  )
  (
    // EVA 32-bit virtual address used by vanilla core
    input [data_width_p-1:0] eva_i  // byte addr
    , input [x_subcord_width_lp-1:0] tgo_x_i // tile-group origin x
    , input [y_subcord_width_lp-1:0] tgo_y_i // tile-group origin y

    , input [pod_x_cord_width_p-1:0] pod_x_i
    , input [pod_y_cord_width_p-1:0] pod_y_i


    // NPA (x,y,EPA)
    , output logic [x_cord_width_p-1:0] x_cord_o  // destination x_cord (global)
    , output logic [y_cord_width_p-1:0] y_cord_o  // destination y_cord (global)
    , output logic [addr_width_p-1:0] epa_o       // endpoint physical address (word addr)

    // EVA does not map to any valid remote NPA location.
    , output logic is_invalid_addr_o
  );

  // localparam
  //
  localparam vcache_word_offset_width_lp = `BSG_SAFE_CLOG2(vcache_block_size_in_words_p);
  localparam lg_vcache_size_lp = `BSG_SAFE_CLOG2(vcache_size_p);
  localparam vcache_row_id_width_lp = `BSG_SAFE_CLOG2(2*num_vcache_rows_p);


  // figure out what type of EVA this is.
  bsg_manycore_global_addr_s global_addr;
  bsg_manycore_tile_group_addr_s tile_group_addr;

  assign global_addr = eva_i;
  assign tile_group_addr = eva_i;

  wire is_dram_addr = eva_i[31];
  wire is_global_addr = global_addr.remote == 2'b01;
  wire is_tile_group_addr = tile_group_addr.remote == 3'b001;


  
  // DRAM hash function
  // DRAM space is striped across vcaches at a cache line granularity.
  // Striping starts from the north vcaches, and alternates between north and south from inner layers to outer layers.
  
  logic [x_cord_width_p-1:0] dram_x_cord_lo;
  logic [y_cord_width_p-1:0] dram_y_cord_lo;
  logic [addr_width_p-1:0] dram_epa_lo;

  bsg_manycore_dram_hash_function #(
    .data_width_p(data_width_p)
    ,.addr_width_p(addr_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.pod_x_cord_width_p(pod_x_cord_width_p)
    ,.pod_y_cord_width_p(pod_y_cord_width_p)
    ,.x_subcord_width_p(x_subcord_width_lp)
    ,.y_subcord_width_p(y_subcord_width_lp)
    ,.num_vcache_rows_p(num_vcache_rows_p)
    ,.vcache_block_size_in_words_p(vcache_block_size_in_words_p)
  ) dram_hash (
    .eva_i(eva_i)
    ,.pod_x_i(pod_x_i)
    ,.pod_y_i(pod_y_i)

    ,.x_cord_o(dram_x_cord_lo)
    ,.y_cord_o(dram_y_cord_lo)
    ,.epa_o(dram_epa_lo)
  );

  // we need a second DRAM hash function
  // just in case that we are accessing a DRAM overlowed DEM addr
  logic [data_width_p-1:0]   eva_overflowed_l;
  logic [x_cord_width_p-1:0] overflowed_dram_x_cord_lo;
  logic [y_cord_width_p-1:0] overflowed_dram_y_cord_lo;
  logic [addr_width_p-1:0]   overflowed_dram_epa_lo;
  assign eva_overflowed_l[data_width_p-1] = 1'b1;
  // assign eva_overflowed_l[data_width_p-2:0] = eva_i[data_width_p-2:0];
  // hardcode vcache_block_size_in_words_p+2-1:0
  // direct assignment
  assign eva_overflowed_l[5:0]   = eva_i[5:0];
  assign eva_overflowed_l[17:15] = eva_i[17:15];
  assign eva_overflowed_l[25:22] = eva_i[25:22];
  assign eva_overflowed_l[30:27] = eva_i[30:27];

  // swap for vcache-x
  // swap for cache index
  assign eva_overflowed_l[9:6]  = eva_i[21:18];
  assign eva_overflowed_l[21:18] = eva_i[14:11];
  assign eva_overflowed_l[14:11] = eva_i[9:6];
  // swap for vcache-y
  assign eva_overflowed_l[10]    = eva_i[26];
  assign eva_overflowed_l[26]    = eva_i[10];

  bsg_manycore_dram_hash_function #(
    .data_width_p(data_width_p)
    ,.addr_width_p(addr_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.pod_x_cord_width_p(pod_x_cord_width_p)
    ,.pod_y_cord_width_p(pod_y_cord_width_p)
    ,.x_subcord_width_p(x_subcord_width_lp)
    ,.y_subcord_width_p(y_subcord_width_lp)
    ,.num_vcache_rows_p(num_vcache_rows_p)
    ,.vcache_block_size_in_words_p(vcache_block_size_in_words_p)
  ) overflowed_dram_hash (
    .eva_i(eva_overflowed_l)
    ,.pod_x_i(pod_x_i)
    ,.pod_y_i(pod_y_i)

    ,.x_cord_o(overflowed_dram_x_cord_lo)
    ,.y_cord_o(overflowed_dram_y_cord_lo)
    ,.epa_o(overflowed_dram_epa_lo)
  );

  // EVA->NPA table
  always_comb begin
    is_invalid_addr_o = 1'b0;

    if (is_dram_addr) begin
      y_cord_o = dram_y_cord_lo;
      x_cord_o = dram_x_cord_lo;
      epa_o = dram_epa_lo;
    end
    else if (is_global_addr) begin
      // global-addr
      // x,y-cord and EPA is directly encoded in EVA.
      y_cord_o = y_cord_width_p'(global_addr.y_cord);
      x_cord_o = x_cord_width_p'(global_addr.x_cord);
      epa_o = {{(addr_width_p-global_epa_word_addr_width_gp){1'b0}}, global_addr.addr};
    end
    else if (is_tile_group_addr) begin
      // DRAM overflowed DMEM Hack:
      // we check if the dmem is in overflowed range or not
      // this is a dmem access made to an addr actually in DRAM
      if (tile_group_addr.addr inside {[16'h0080:16'hFC7F]}) begin
        y_cord_o = overflowed_dram_y_cord_lo;
        x_cord_o = overflowed_dram_x_cord_lo;
        epa_o = overflowed_dram_epa_lo;
        // $display("[INFO][LC] possible overflowed DMEM access t=%0t; x=%d, y=%d; addr=%h; overflowed addr=%h; vcache y=%d x=%d; epa_o=%h", $time, tile_group_addr.x_cord, tile_group_addr.y_cord, eva_i, eva_overflowed_l, y_cord_o, x_cord_o, overflowed_dram_epa_lo);
      end
      else begin
        // tile-group addr
        // tile-coordinate in the EVA is added to the tile-group origin register.
        y_cord_o = {pod_y_i, y_subcord_width_lp'(tile_group_addr.y_cord + tgo_y_i)};
        x_cord_o = {pod_x_i, x_subcord_width_lp'(tile_group_addr.x_cord + tgo_x_i)};
        // here we do an and with 0x03FF to make sure the addr is a valid physical dmem addr
        // XXX: LC - make sure this mask is correct!
        epa_o = {{(addr_width_p-tile_group_epa_word_addr_width_gp){1'b0}}, (tile_group_addr.addr & 16'h03FF)};
      end
    end
    else begin
      // should never happen
      y_cord_o = '0;
      x_cord_o = '0;
      epa_o = '0;
      is_invalid_addr_o = 1'b1;
    end
  end


endmodule

`BSG_ABSTRACT_MODULE(bsg_manycore_eva_to_npa)
