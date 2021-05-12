/**
 *    bsg_nonsynth_manycore_testbench.v
 *
 */


module bsg_nonsynth_manycore_testbench
  import bsg_noc_pkg::*; // {P=0, W, E, N, S}
  import bsg_tag_pkg::*;
  import bsg_manycore_pkg::*;
  import bsg_manycore_mem_cfg_pkg::*;
  #(parameter num_pods_x_p  = "inv"
    , parameter num_pods_y_p  = "inv"
    , parameter num_tiles_x_p = "inv"
    , parameter num_tiles_y_p = "inv"
    , parameter x_cord_width_p = "inv"
    , parameter y_cord_width_p = "inv"
    , parameter pod_x_cord_width_p = "inv"
    , parameter pod_y_cord_width_p = "inv"
    , parameter addr_width_p = "inv"
    , parameter data_width_p = "inv"
    , parameter dmem_size_p = "inv"
    , parameter icache_entries_p = "inv"
    , parameter icache_tag_width_p = "inv"
    , parameter ruche_factor_X_p  = "inv"

    , parameter num_subarray_x_p = "inv"
    , parameter num_subarray_y_p = "inv"

    , parameter num_vcache_rows_p = "inv"
    , parameter vcache_data_width_p = "inv"
    , parameter vcache_sets_p = "inv"
    , parameter vcache_ways_p = "inv"
    , parameter vcache_block_size_in_words_p = "inv" // in words
    , parameter vcache_dma_data_width_p = "inv" // in bits
    , parameter vcache_size_p = "inv" // in words
    , parameter vcache_addr_width_p="inv" // byte addr
    , parameter num_vcaches_per_channel_p = "inv"

    , parameter wh_flit_width_p = "inv"
    , parameter wh_ruche_factor_p = 2
    , parameter wh_cid_width_p = "inv"
    , parameter wh_len_width_p = "inv"
    , parameter wh_cord_width_p = "inv"

    , parameter bsg_manycore_mem_cfg_e bsg_manycore_mem_cfg_p = e_vcache_test_mem
    , parameter bsg_dram_size_p ="inv" // in word

    , parameter bsg_manycore_composition = "inv"

    , parameter reset_depth_p = 3

    , parameter enable_vcore_profiling_p=0
    , parameter enable_router_profiling_p=0
    , parameter enable_cache_profiling_p=0

    , parameter cache_bank_addr_width_lp = `BSG_SAFE_CLOG2(bsg_dram_size_p/(2*num_tiles_x_p*num_vcache_rows_p)*4) // byte addr
    , parameter link_sif_width_lp =
      `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)

    // This is used to define heterogeneous arrays. Each index defines
    // the type of an X/Y coordinate in the array. This is a vector of
    // num_tiles_x_p*num_tiles_y_p ints; type "0" is the
    // default. See bsg_manycore_hetero_socket.v for more types.
    , parameter int hetero_type_vec_p [0:(num_tiles_y_p*num_tiles_x_p) - 1]  = '{default:0}

    , localparam sdr_lg_fifo_depth_gp = 3
    , localparam sdr_lg_credit_to_token_decimation_gp = 0

    , localparam x_subcord_width_lp=`BSG_SAFE_CLOG2(num_tiles_x_p)
    , localparam y_subcord_width_lp=`BSG_SAFE_CLOG2(num_tiles_y_p)

    , localparam fwd_width_lp =
      `bsg_manycore_packet_width(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p)
    , localparam rev_width_lp =
      `bsg_manycore_return_packet_width(x_cord_width_p, y_cord_width_p, data_width_p)
  )
  (
    input clk_i
    , input cgra_xcel_clk_i
    , input reset_i

    , output tag_done_o
    
    , input  [link_sif_width_lp-1:0] io_link_sif_i
    , output [link_sif_width_lp-1:0] io_link_sif_o
  );


  // print machine settings
  initial begin
    $display("MACHINE SETTINGS:");
    $display("[INFO][TESTBENCH] BSG_MACHINE_GLOBAL_X                 = %d", num_tiles_x_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_GLOBAL_Y                 = %d", num_tiles_y_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_VCACHE_SET               = %d", vcache_sets_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_VCACHE_WAY               = %d", vcache_ways_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_VCACHE_BLOCK_SIZE_WORDS  = %d", vcache_block_size_in_words_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_MAX_EPA_WIDTH            = %d", addr_width_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_MEM_CFG                  = %s", bsg_manycore_mem_cfg_p.name());
    $display("[INFO][TESTBENCH] BSG_MACHINE_RUCHE_FACTOR_X           = %d", ruche_factor_X_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_SUBARRAY_X               = %d", num_subarray_x_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_SUBARRAY_Y               = %d", num_subarray_y_p);
    $display("[INFO][TESTBENCH] BSG_MACHINE_ORIGIN_X_CORD            = %d", `BSG_MACHINE_ORIGIN_X_CORD);
    $display("[INFO][TESTBENCH] BSG_MACHINE_ORIGIN_Y_CORD            = %d", `BSG_MACHINE_ORIGIN_Y_CORD);
    $display("[INFO][TESTBENCH] BSG_MACHINE_COMPOSITION              = %d", bsg_manycore_composition);
    $display("[INFO][TESTBENCH] BSG_MACHINE_NUM_VCACHE_ROWS          = %d", num_vcache_rows_p);
    $display("[INFO][TESTBENCH] enable_vcore_profiling_p             = %d", enable_vcore_profiling_p);
    $display("[INFO][TESTBENCH] enable_router_profiling_p            = %d", enable_router_profiling_p);
    $display("[INFO][TESTBENCH] enable_cache_profiling_p             = %d", enable_cache_profiling_p);
  end

  logic async_uplink_reset, async_downlink_reset, async_downstream_reset, async_token_reset;
  logic reset_done;
  initial
    begin
      @(negedge reset_i);

      $display("[INFO][SDR-Reset] Start SDR reset sequence at time ", $stime);

      async_uplink_reset     = 1'b1;
      async_downlink_reset   = 1'b1;
      async_downstream_reset = 1'b1;
      async_token_reset      = 1'b0;
      reset_done             = 1'b0;

      #100000;
      async_token_reset = 1'b1;
      #100000;
      async_token_reset = 1'b0;
      #100000;
      async_uplink_reset = 1'b0;
      #100000;
      async_downlink_reset = 1'b0;
      #100000;
      async_downstream_reset = 1'b0;
      #100000;
      reset_done = 1'b1;

      $display("[INFO][SDR-Reset] SDR reset sequence finished at time ", $stime);
    end

  // BSG TAG MASTER
  logic tag_done_lo;
  bsg_tag_s [num_pods_y_p-1:0][num_pods_x_p-1:0] pod_tags_lo;

  bsg_nonsynth_manycore_tag_master #(
    .num_pods_x_p(num_pods_x_p)
    ,.num_pods_y_p(num_pods_y_p)
    ,.wh_cord_width_p(wh_cord_width_p)
  ) mtm (
    .clk_i(clk_i)
    // PP: only start tag programming after SDR reset has finished
    ,.reset_i(reset_i | ~reset_done)
    
    ,.tag_done_o(tag_done_lo)
    ,.pod_tags_o(pod_tags_lo)
  );   
  
  assign tag_done_o = tag_done_lo;

  //---------------------------------------------------------------------
  // BSG tag master for the CGRA half pod
  //---------------------------------------------------------------------

  // localparam cgra_tag_num_clients_lp = 4;
  // localparam cgra_tag_payload_width_lp = y_cord_width_p;
  // localparam cgra_tag_lg_payload_width_lp = `BSG_WIDTH(cgra_tag_payload_width_lp);
  // localparam cgra_tag_max_payload_width_lp = (1<<cgra_tag_lg_payload_width_lp)-1;
  // localparam cgra_tag_rom_data_width_lp = 4+1+`BSG_SAFE_CLOG2(cgra_tag_num_clients_lp)+1+cgra_tag_lg_payload_width_lp+cgra_tag_max_payload_width_lp;
  // localparam cgra_tag_rom_addr_width_lp = 12;

  // logic cgra_tag_tr_valid_lo, cgra_tag_tr_data_lo, cgra_tag_tr_done_lo;
  // logic [cgra_tag_rom_data_width_lp-1:0] cgra_tag_rom_data;
  // logic [cgra_tag_rom_addr_width_lp-1:0] cgra_tag_rom_addr;

  // logic cgra_tag_done_lo;
  // bsg_tag_s [num_pods_y_p-1:0][cgra_tag_num_clients_lp-1:0] cgra_tags_lo;

  // bsg_tag_trace_replay
  //  #(.rom_addr_width_p(cgra_tag_rom_addr_width_lp)
  //    ,.rom_data_width_p(cgra_tag_rom_data_width_lp)
  //    ,.num_clients_p(cgra_tag_num_clients_lp)
  //    ,.max_payload_width_p(cgra_tag_max_payload_width_lp)
  //    ,.num_masters_p(1)
  //    )
  //  cgra_tag_tr
  //   (.clk_i(clk_i)
  //    ,.reset_i(reset_i)
  //    ,.en_i(1'b1)

  //    ,.rom_addr_o(cgra_tag_rom_addr)
  //    ,.rom_data_i(cgra_tag_rom_data)

  //    ,.valid_i(1'b0)
  //    ,.data_i('0)
  //    ,.ready_o()

  //    ,.valid_o(cgra_tag_tr_valid_lo)
  //    ,.en_r_o()
  //    ,.tag_data_o(cgra_tag_tr_data_lo)
  //    ,.yumi_i(cgra_tag_tr_valid_lo)

  //    ,.done_o(cgra_tag_tr_done_lo)
  //    ,.error_o()
  //    );

  // cgra_hpod_rom #(
  //   .width_p(cgra_tag_rom_data_width_lp)
  //   ,.addr_width_p(cgra_tag_rom_addr_width_lp)
  // ) hpod_rom (
  //   .addr_i(cgra_tag_rom_addr)
  //   ,.data_o(cgra_tag_rom_data)
  // );

  // bsg_tag_master
  //  #(.els_p(cgra_tag_num_clients_lp), .lg_width_p(cgra_tag_lg_payload_width_lp))
  //  cgra_tag_btm
  //   (.clk_i(clk_i)
  //    ,.data_i(cgra_tag_tr_valid_lo & cgra_tag_tr_data_lo)
  //    ,.en_i(1'b1)
  //    ,.clients_r_o(cgra_tags_lo)
  //   );

  //---------------------------------------------------------------------
  // deassert reset when tag programming is done.
  wire reset = ~tag_done_lo;
  logic reset_r;
  bsg_dff_chain #(
    .width_p(1)
    ,.num_stages_p(reset_depth_p)
  ) reset_dff (
    .clk_i(clk_i)
    ,.data_i(reset)
    ,.data_o(reset_r)
  );

  // instantiate manycore
  `declare_bsg_manycore_link_sif_s(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p);
  `declare_bsg_manycore_ruche_x_link_sif_s(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p);
  `declare_bsg_ready_and_link_sif_s(wh_flit_width_p, wh_link_sif_s);
  bsg_manycore_link_sif_s [S:N][(num_pods_x_p*num_tiles_x_p)-1:0] ver_link_sif_li;
  bsg_manycore_link_sif_s [S:N][(num_pods_x_p*num_tiles_x_p)-1:0] ver_link_sif_lo;
  wh_link_sif_s [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0] wh_link_sif_li;
  wh_link_sif_s [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0] wh_link_sif_lo;
  bsg_manycore_link_sif_s [E:W][num_pods_y_p-1:0][num_tiles_y_p-1:0] hor_link_sif_li;
  bsg_manycore_link_sif_s [E:W][num_pods_y_p-1:0][num_tiles_y_p-1:0] hor_link_sif_lo;
  bsg_manycore_ruche_x_link_sif_s [E:W][num_pods_y_p-1:0][num_tiles_y_p-1:0] ruche_link_li;
  bsg_manycore_ruche_x_link_sif_s [E:W][num_pods_y_p-1:0][num_tiles_y_p-1:0] ruche_link_lo;

  bsg_manycore_pod_ruche_array #(
    .num_tiles_x_p(num_tiles_x_p)
    ,.num_tiles_y_p(num_tiles_y_p)
    ,.pod_x_cord_width_p(pod_x_cord_width_p)
    ,.pod_y_cord_width_p(pod_y_cord_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.ruche_factor_X_p(ruche_factor_X_p)

    ,.num_subarray_x_p(num_subarray_x_p)
    ,.num_subarray_y_p(num_subarray_y_p)

    ,.dmem_size_p(dmem_size_p)
    ,.icache_entries_p(icache_entries_p)
    ,.icache_tag_width_p(icache_tag_width_p)

    ,.num_vcache_rows_p(num_vcache_rows_p)
    ,.vcache_addr_width_p(vcache_addr_width_p)
    ,.vcache_data_width_p(vcache_data_width_p)
    ,.vcache_ways_p(vcache_ways_p)
    ,.vcache_sets_p(vcache_sets_p)
    ,.vcache_block_size_in_words_p(vcache_block_size_in_words_p)
    ,.vcache_size_p(vcache_size_p)
    ,.vcache_dma_data_width_p(vcache_dma_data_width_p)

    ,.wh_ruche_factor_p(wh_ruche_factor_p)
    ,.wh_cid_width_p(wh_cid_width_p)
    ,.wh_flit_width_p(wh_flit_width_p)
    ,.wh_cord_width_p(wh_cord_width_p)
    ,.wh_len_width_p(wh_len_width_p)

    ,.num_pods_y_p(num_pods_y_p)
    ,.num_pods_x_p(num_pods_x_p)

    ,.reset_depth_p(reset_depth_p)
    ,.hetero_type_vec_p(hetero_type_vec_p)
  ) DUT (
    .clk_i(clk_i)

    ,.ver_link_sif_i(ver_link_sif_li)
    ,.ver_link_sif_o(ver_link_sif_lo)

    ,.wh_link_sif_i(wh_link_sif_li)
    ,.wh_link_sif_o(wh_link_sif_lo)

    ,.hor_link_sif_i(hor_link_sif_li)
    ,.hor_link_sif_o(hor_link_sif_lo)

    ,.ruche_link_i(ruche_link_li)
    ,.ruche_link_o(ruche_link_lo)

    ,.pod_tags_i(pod_tags_lo) 
  );

  // Invert WH ruche links
  // hardcoded for ruche factor = 2
  wh_link_sif_s [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0] buffered_wh_link_sif_li;
  wh_link_sif_s [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0] buffered_wh_link_sif_lo;
  for (genvar i = W; i <= E; i++) begin
    for (genvar j = 0; j < num_pods_y_p; j++) begin
      for (genvar k = N; k <= S; k++) begin
        for (genvar v = 0; v < num_vcache_rows_p; v++) begin
          for (genvar r = 0; r < wh_ruche_factor_p; r++) begin
            if (r == 0) begin
              assign wh_link_sif_li[i][j][k][v][r] = buffered_wh_link_sif_li[i][j][k][v][r];
              assign buffered_wh_link_sif_lo[i][j][k][v][r] = wh_link_sif_lo[i][j][k][v][r];
            end
            else begin
              assign wh_link_sif_li[i][j][k][v][r] = ~buffered_wh_link_sif_li[i][j][k][v][r];
              assign buffered_wh_link_sif_lo[i][j][k][v][r] = ~wh_link_sif_lo[i][j][k][v][r];
            end
          end
        end
      end
    end
  end

  // IO ROUTER
  localparam rev_use_credits_lp = 5'b00001;
  localparam int rev_fifo_els_lp[4:0] = '{2,2,2,2,3};
  bsg_manycore_link_sif_s [(num_pods_x_p*num_tiles_x_p)-1:0][S:P] io_link_sif_li;
  bsg_manycore_link_sif_s [(num_pods_x_p*num_tiles_x_p)-1:0][S:P] io_link_sif_lo;

  for (genvar x = 0; x < num_pods_x_p*num_tiles_x_p; x++) begin: io_rtr_x
    bsg_manycore_mesh_node #(
      .x_cord_width_p(x_cord_width_p)
      ,.y_cord_width_p(y_cord_width_p)
      ,.addr_width_p(addr_width_p)
      ,.data_width_p(data_width_p)
      ,.stub_p(4'b0100) // stub north
      ,.rev_use_credits_p(rev_use_credits_lp)
      ,.rev_fifo_els_p(rev_fifo_els_lp)
    ) io_rtr (
      .clk_i(clk_i)
      ,.reset_i(reset_r)

      ,.links_sif_i(io_link_sif_li[x][S:W])
      ,.links_sif_o(io_link_sif_lo[x][S:W])

      ,.proc_link_sif_i(io_link_sif_li[x][P])
      ,.proc_link_sif_o(io_link_sif_lo[x][P])

      ,.global_x_i(x_cord_width_p'(num_tiles_x_p+x))
      ,.global_y_i(y_cord_width_p'(0))
    );

    // connect to pod array
    assign ver_link_sif_li[N][x] = io_link_sif_lo[x][S];
    assign io_link_sif_li[x][S] = ver_link_sif_lo[N][x];

    // connect between io rtr
    if (x < (num_pods_x_p*num_tiles_x_p)-1) begin
      assign io_link_sif_li[x][E] = io_link_sif_lo[x+1][W];
      assign io_link_sif_li[x+1][W] = io_link_sif_lo[x][E];
    end
  end



  // Host link connection
  assign io_link_sif_li[0][P] = io_link_sif_i;
  assign io_link_sif_o = io_link_sif_lo[0][P];




  //                              //
  // Configurable Memory System   //
  //                              //
  localparam logic [e_max_val-1:0] mem_cfg_lp = (1 << bsg_manycore_mem_cfg_p);

  if (mem_cfg_lp[e_vcache_test_mem]) begin: test_mem
    // in bytes
    // north + south row of vcache
    localparam longint unsigned mem_size_lp = (2**30)*num_pods_x_p/wh_ruche_factor_p/num_vcache_rows_p/2;
    localparam num_vcaches_per_test_mem_lp = (num_tiles_x_p*num_pods_x_p)/wh_ruche_factor_p/2;

    for (genvar i = W; i <= E; i++) begin: hs                           // horizontal side
      for (genvar j = 0; j < num_pods_y_p; j++) begin: py               // pod y
        for (genvar k = N; k <= S; k++) begin: vs                       // vertical side
          for (genvar v = 0; v < num_vcache_rows_p; v++) begin: vr      // vcache row
            for (genvar r = 0; r < wh_ruche_factor_p; r++) begin: rf    // ruching
              bsg_nonsynth_wormhole_test_mem #(
                .vcache_data_width_p(vcache_data_width_p)
                ,.vcache_dma_data_width_p(vcache_dma_data_width_p)
                ,.vcache_block_size_in_words_p(vcache_block_size_in_words_p)
                ,.num_vcaches_p(num_vcaches_per_test_mem_lp)
                ,.wh_cid_width_p(wh_cid_width_p)
                ,.wh_flit_width_p(wh_flit_width_p)
                ,.wh_cord_width_p(wh_cord_width_p)
                ,.wh_len_width_p(wh_len_width_p)
                ,.wh_ruche_factor_p(wh_ruche_factor_p)
                ,.no_concentration_p(1)
                ,.mem_size_p(mem_size_lp)
              ) test_mem (
                .clk_i(clk_i)
                ,.reset_i(reset_r)

                ,.wh_link_sif_i(buffered_wh_link_sif_lo[i][j][k][v][r])
                ,.wh_link_sif_o(buffered_wh_link_sif_li[i][j][k][v][r])
              );
            end
          end
        end
      end
    end

  end
  else if (mem_cfg_lp[e_vcache_hbm2]) begin: hbm2
    

    `define dram_pkg `BSG_MACHINE_DRAMSIM3_PKG
    parameter hbm2_data_width_p = `dram_pkg::data_width_p;
    parameter hbm2_channel_addr_width_p = `dram_pkg::channel_addr_width_p;
    parameter hbm2_num_channels_p = `dram_pkg::num_channels_p;
      
    parameter num_total_vcaches_lp = (num_pods_x_p*num_pods_y_p*2*num_tiles_x_p*num_vcache_rows_p);
    parameter lg_num_total_vcaches_lp = `BSG_SAFE_CLOG2(num_total_vcaches_lp);
    parameter num_vcaches_per_link_lp = (num_tiles_x_p*num_pods_x_p)/wh_ruche_factor_p/2; // # of vcaches attached to each link

    parameter num_total_channels_lp = num_total_vcaches_lp/num_vcaches_per_channel_p;
    parameter num_dram_lp = `BSG_CDIV(num_total_channels_lp,hbm2_num_channels_p);


    // WH to cache dma
    `declare_bsg_cache_dma_pkt_s(vcache_addr_width_p);
    bsg_cache_dma_pkt_s [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0] dma_pkt_lo;
    logic [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0] dma_pkt_v_lo;
    logic [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0] dma_pkt_yumi_li;

    logic [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0][vcache_dma_data_width_p-1:0] dma_data_li;
    logic [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0] dma_data_v_li;
    logic [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0] dma_data_ready_lo;

    logic [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0][vcache_dma_data_width_p-1:0] dma_data_lo;
    logic [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0] dma_data_v_lo;
    logic [E:W][num_pods_y_p-1:0][S:N][num_vcache_rows_p-1:0][wh_ruche_factor_p-1:0][num_vcaches_per_link_lp-1:0] dma_data_yumi_li;


    for (genvar i = W; i <= E; i++) begin: hs
      for (genvar j = 0; j < num_pods_y_p; j++) begin: py
        for (genvar k = N; k <= S; k++) begin: py
          for (genvar n = 0; n < num_vcache_rows_p; n++) begin: row
            for (genvar r = 0; r < wh_ruche_factor_p; r++) begin: rf
              bsg_manycore_vcache_wh_to_cache_dma #(
               .wh_flit_width_p(wh_flit_width_p)
               ,.wh_cid_width_p(wh_cid_width_p)
                ,.wh_len_width_p(wh_len_width_p)
                ,.wh_cord_width_p(wh_cord_width_p)
                ,.wh_ruche_factor_p(wh_ruche_factor_p)

                ,.num_vcaches_p(num_vcaches_per_link_lp)
                ,.vcache_addr_width_p(vcache_addr_width_p)
                ,.vcache_data_width_p(vcache_data_width_p)
                ,.vcache_dma_data_width_p(vcache_dma_data_width_p)
                ,.vcache_block_size_in_words_p(vcache_block_size_in_words_p)

                ,.no_concentration_p(1)
                ,.num_pods_x_p(num_pods_x_p)
                ,.pod_start_x_p(1)
                ,.num_tiles_x_p(num_tiles_x_p)
              ) wh_to_dma (
                .clk_i(clk_i)
                ,.reset_i(reset_r)
    
                ,.wh_link_sif_i     (buffered_wh_link_sif_lo[i][j][k][n][r])
                ,.wh_link_sif_o     (buffered_wh_link_sif_li[i][j][k][n][r])

                ,.dma_pkt_o         (dma_pkt_lo[i][j][k][n][r])
                ,.dma_pkt_v_o       (dma_pkt_v_lo[i][j][k][n][r])
                ,.dma_pkt_yumi_i    (dma_pkt_yumi_li[i][j][k][n][r])

                ,.dma_data_i        (dma_data_li[i][j][k][n][r])
                ,.dma_data_v_i      (dma_data_v_li[i][j][k][n][r])
                ,.dma_data_ready_o  (dma_data_ready_lo[i][j][k][n][r])

                ,.dma_data_o        (dma_data_lo[i][j][k][n][r])
                ,.dma_data_v_o      (dma_data_v_lo[i][j][k][n][r])
                ,.dma_data_yumi_i   (dma_data_yumi_li[i][j][k][n][r])
              );
            end
          end
        end
      end
    end


    // cache DMA to DRAMSIM3
    // assign vcache DMA to correct HBM2 channel / bank
    bsg_cache_dma_pkt_s [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0] remapped_dma_pkt_lo;
    logic [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0] remapped_dma_pkt_v_lo;
    logic [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0] remapped_dma_pkt_yumi_li;

    logic [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0][vcache_dma_data_width_p-1:0] remapped_dma_data_li;
    logic [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0] remapped_dma_data_v_li;
    logic [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0] remapped_dma_data_ready_lo;

    logic [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0][vcache_dma_data_width_p-1:0] remapped_dma_data_lo;
    logic [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0] remapped_dma_data_v_lo;
    logic [num_total_channels_lp-1:0][num_vcaches_per_channel_p-1:0] remapped_dma_data_yumi_li;


    vcache_dma_to_dram_channel_map #(
      .num_pods_y_p(num_pods_y_p)
      ,.num_pods_x_p(num_pods_x_p)
      ,.num_tiles_x_p(num_tiles_x_p)

      ,.wh_ruche_factor_p(wh_ruche_factor_p)

      ,.num_vcache_rows_p(num_vcache_rows_p)
      ,.vcache_addr_width_p(vcache_addr_width_p)
      ,.vcache_dma_data_width_p(vcache_dma_data_width_p)
    ) dma_map (
      // unmapped
      .dma_pkt_i                    (dma_pkt_lo)
      ,.dma_pkt_v_i                 (dma_pkt_v_lo)
      ,.dma_pkt_yumi_o              (dma_pkt_yumi_li)

      ,.dma_data_o                  (dma_data_li)
      ,.dma_data_v_o                (dma_data_v_li)
      ,.dma_data_ready_i            (dma_data_ready_lo)

      ,.dma_data_i                  (dma_data_lo)
      ,.dma_data_v_i                (dma_data_v_lo)
      ,.dma_data_yumi_o             (dma_data_yumi_li)

      // remapped
      ,.remapped_dma_pkt_o          (remapped_dma_pkt_lo)
      ,.remapped_dma_pkt_v_o        (remapped_dma_pkt_v_lo)
      ,.remapped_dma_pkt_yumi_i     (remapped_dma_pkt_yumi_li)
      
      ,.remapped_dma_data_i         (remapped_dma_data_li)
      ,.remapped_dma_data_v_i       (remapped_dma_data_v_li)
      ,.remapped_dma_data_ready_o   (remapped_dma_data_ready_lo)

      ,.remapped_dma_data_o         (remapped_dma_data_lo)
      ,.remapped_dma_data_v_o       (remapped_dma_data_v_lo)
      ,.remapped_dma_data_yumi_i    (remapped_dma_data_yumi_li)
    );
        

    // DRAMSIM3
    logic [(num_dram_lp*hbm2_num_channels_p)-1:0] dramsim3_v_li;
    logic [(num_dram_lp*hbm2_num_channels_p)-1:0] dramsim3_write_not_read_li;
    logic [(num_dram_lp*hbm2_num_channels_p)-1:0][hbm2_channel_addr_width_p-1:0] dramsim3_ch_addr_li;
    logic [(num_dram_lp*hbm2_num_channels_p)-1:0] dramsim3_yumi_lo;

    logic [(num_dram_lp*hbm2_num_channels_p)-1:0][hbm2_data_width_p-1:0] dramsim3_data_li;
    logic [(num_dram_lp*hbm2_num_channels_p)-1:0] dramsim3_data_v_li;
    logic [(num_dram_lp*hbm2_num_channels_p)-1:0] dramsim3_data_yumi_lo;

    logic [(num_dram_lp*hbm2_num_channels_p)-1:0][hbm2_data_width_p-1:0] dramsim3_data_lo;
    logic [(num_dram_lp*hbm2_num_channels_p)-1:0] dramsim3_data_v_lo;
    `dram_pkg::dram_ch_addr_s [(num_dram_lp*hbm2_num_channels_p)-1:0] dramsim3_read_done_ch_addr_lo;
    
    for (genvar i = 0; i < num_dram_lp; i++) begin
      bsg_nonsynth_dramsim3 #(
        .channel_addr_width_p (hbm2_channel_addr_width_p)
        ,.data_width_p        (hbm2_data_width_p)
        ,.num_channels_p      (hbm2_num_channels_p)
        ,.num_columns_p       (`dram_pkg::num_columns_p)
        ,.num_rows_p          (`dram_pkg::num_rows_p)
        ,.num_ba_p            (`dram_pkg::num_ba_p)
        ,.num_bg_p            (`dram_pkg::num_bg_p)
        ,.num_ranks_p         (`dram_pkg::num_ranks_p)
        ,.address_mapping_p   (`dram_pkg::address_mapping_p)
        ,.size_in_bits_p      (`dram_pkg::size_in_bits_p)
        ,.config_p            (`dram_pkg::config_p)
        ,.init_mem_p          (1)
        ,.base_id_p           (i*hbm2_num_channels_p)
      ) hbm0 (
        .clk_i                (clk_i)
        ,.reset_i             (reset_r)
      
        ,.v_i                 (dramsim3_v_li[hbm2_num_channels_p*i+:hbm2_num_channels_p])
        ,.write_not_read_i    (dramsim3_write_not_read_li[hbm2_num_channels_p*i+:hbm2_num_channels_p])
        ,.ch_addr_i           (dramsim3_ch_addr_li[hbm2_num_channels_p*i+:hbm2_num_channels_p])
        ,.mask_i              ('1)
        ,.yumi_o              (dramsim3_yumi_lo[hbm2_num_channels_p*i+:hbm2_num_channels_p])

        ,.data_v_i            (dramsim3_data_v_li[hbm2_num_channels_p*i+:hbm2_num_channels_p])
        ,.data_i              (dramsim3_data_li[hbm2_num_channels_p*i+:hbm2_num_channels_p])
        ,.data_yumi_o         (dramsim3_data_yumi_lo[hbm2_num_channels_p*i+:hbm2_num_channels_p])

        ,.data_v_o            (dramsim3_data_v_lo[hbm2_num_channels_p*i+:hbm2_num_channels_p])
        ,.data_o              (dramsim3_data_lo[hbm2_num_channels_p*i+:hbm2_num_channels_p])
        ,.read_done_ch_addr_o (dramsim3_read_done_ch_addr_lo[hbm2_num_channels_p*i+:hbm2_num_channels_p])

        ,.print_stat_v_i      ($root.`HOST_MODULE_PATH.print_stat_v)
        ,.print_stat_tag_i    ($root.`HOST_MODULE_PATH.print_stat_tag)

        ,.write_done_o        ()
        ,.write_done_ch_addr_o()
      );
    end


    // cache to test dram
    // This is the address format coming out of cache dma.
    typedef struct packed {
      logic [$clog2(`dram_pkg::num_ba_p)-1:0] ba;
      logic [$clog2(`dram_pkg::num_bg_p)-1:0] bg;
      logic [$clog2(`dram_pkg::num_rows_p)-1:0] ro;
      logic [$clog2(`dram_pkg::num_columns_p)-1:0] co;
      logic [$clog2(`dram_pkg::data_width_p>>3)-1:0] byte_offset;
    } dram_ch_addr_s; 

    dram_ch_addr_s [num_total_channels_lp-1:0] test_dram_ch_addr_lo;
    logic [num_total_channels_lp-1:0][hbm2_channel_addr_width_p-1:0] test_dram_ch_addr_li;

    for (genvar i = 0; i < num_total_channels_lp; i++) begin

      bsg_cache_to_test_dram #(
        .num_cache_p(num_vcaches_per_channel_p)
        ,.addr_width_p(vcache_addr_width_p)
        ,.data_width_p(vcache_data_width_p)
        ,.block_size_in_words_p(vcache_block_size_in_words_p)
        ,.cache_bank_addr_width_p(cache_bank_addr_width_lp)
        ,.dma_data_width_p(vcache_dma_data_width_p)
      
        ,.dram_channel_addr_width_p(hbm2_channel_addr_width_p)
        ,.dram_data_width_p(hbm2_data_width_p)
      ) cache_to_tram (
        .core_clk_i           (clk_i)
        ,.core_reset_i        (reset_r)

        ,.dma_pkt_i           (remapped_dma_pkt_lo[i])
        ,.dma_pkt_v_i         (remapped_dma_pkt_v_lo[i])
        ,.dma_pkt_yumi_o      (remapped_dma_pkt_yumi_li[i])

        ,.dma_data_o          (remapped_dma_data_li[i])
        ,.dma_data_v_o        (remapped_dma_data_v_li[i])
        ,.dma_data_ready_i    (remapped_dma_data_ready_lo[i])

        ,.dma_data_i          (remapped_dma_data_lo[i])
        ,.dma_data_v_i        (remapped_dma_data_v_lo[i])
        ,.dma_data_yumi_o     (remapped_dma_data_yumi_li[i])


        ,.dram_clk_i              (clk_i)
        ,.dram_reset_i            (reset_r)
    
        ,.dram_req_v_o            (dramsim3_v_li[i])
        ,.dram_write_not_read_o   (dramsim3_write_not_read_li[i])
        ,.dram_ch_addr_o          (test_dram_ch_addr_lo[i])
        ,.dram_req_yumi_i         (dramsim3_yumi_lo[i])

        ,.dram_data_v_o           (dramsim3_data_v_li[i])
        ,.dram_data_o             (dramsim3_data_li[i])
        ,.dram_data_yumi_i        (dramsim3_data_yumi_lo[i])

        ,.dram_data_v_i           (dramsim3_data_v_lo[i])
        ,.dram_data_i             (dramsim3_data_lo[i])
        ,.dram_ch_addr_i          (test_dram_ch_addr_li[i])
      );

      // manycore to dramsim3 address hashing
      // dramsim3 uses ro-bg-ba-co-bo as address map, so we are changing the mapping here.
      assign dramsim3_ch_addr_li[i] = {
        test_dram_ch_addr_lo[i].ro,
        test_dram_ch_addr_lo[i].bg,
        test_dram_ch_addr_lo[i].ba,
        test_dram_ch_addr_lo[i].co,
        test_dram_ch_addr_lo[i].byte_offset
      };

      // dramsim3 to manycore address hashing
      // address coming out of dramsim3 is also ro-bg-ba-co-bo, so we are changing it back to the format that cache dma uses.
      assign test_dram_ch_addr_li[i] = {
        dramsim3_read_done_ch_addr_lo[i].ba,
        dramsim3_read_done_ch_addr_lo[i].bg,
        dramsim3_read_done_ch_addr_lo[i].ro,
        dramsim3_read_done_ch_addr_lo[i].co,
        dramsim3_read_done_ch_addr_lo[i].byte_offset
      };
    end
  end













  ////                        ////
  ////      TIE OFF           ////
  ////                        ////


  // IO P tie off
  for (genvar i = 1; i < num_pods_x_p*num_tiles_x_p; i++) begin
    bsg_manycore_link_sif_tieoff #(
      .addr_width_p(addr_width_p)
      ,.data_width_p(data_width_p)
      ,.x_cord_width_p(x_cord_width_p)
      ,.y_cord_width_p(y_cord_width_p)
    ) io_p_tieoff (
      .clk_i(clk_i)
      ,.reset_i(reset_r)
      ,.link_sif_i(io_link_sif_lo[i][P])
      ,.link_sif_o(io_link_sif_li[i][P])
    );
  end

  // IO west end tieoff
  bsg_manycore_link_sif_tieoff #(
    .addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
  ) io_w_tieoff (
    .clk_i(clk_i)
    ,.reset_i(reset_r)
    ,.link_sif_i(io_link_sif_lo[0][W])
    ,.link_sif_o(io_link_sif_li[0][W])
  );

  // IO east end tieoff
  bsg_manycore_link_sif_tieoff #(
    .addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
  ) io_e_tieoff (
    .clk_i(clk_i)
    ,.reset_i(reset_r)
    ,.link_sif_i(io_link_sif_lo[(num_pods_x_p*num_tiles_x_p)-1][E])
    ,.link_sif_o(io_link_sif_li[(num_pods_x_p*num_tiles_x_p)-1][E])
  );


  // SOUTH VER LINK TIE OFFS
  for (genvar i = 0; i < num_pods_x_p*num_tiles_x_p; i++) begin
    bsg_manycore_link_sif_tieoff #(
      .addr_width_p(addr_width_p)
      ,.data_width_p(data_width_p)
      ,.x_cord_width_p(x_cord_width_p)
      ,.y_cord_width_p(y_cord_width_p)
    ) ver_s_tieoff (
      .clk_i(clk_i)
      ,.reset_i(reset_r)
      ,.link_sif_i(ver_link_sif_lo[S][i])
      ,.link_sif_o(ver_link_sif_li[S][i])
    );
  end


  // HOR TIEOFF (west)
  for (genvar j = 0; j < num_pods_y_p; j++) begin
    for (genvar k = 0; k < num_tiles_y_p; k++) begin
      bsg_manycore_link_sif_tieoff #(
        .addr_width_p(addr_width_p)
        ,.data_width_p(data_width_p)
        ,.x_cord_width_p(x_cord_width_p)
        ,.y_cord_width_p(y_cord_width_p)
      ) hor_w_tieoff (
        .clk_i(clk_i)
        ,.reset_i(reset_r)
        ,.link_sif_i(hor_link_sif_lo[W][j][k])
        ,.link_sif_o(hor_link_sif_li[W][j][k])
      );
    end
  end

  // HOR TIEOFF (east)
  if (bsg_manycore_composition == e_manycore) begin: mc_tieoff
    // PP: This is a pure manycore composition. Tie off east HOR links
    for (genvar j = 0; j < num_pods_y_p; j++) begin
      for (genvar k = 0; k < num_tiles_y_p; k++) begin
        bsg_manycore_link_sif_tieoff #(
          .addr_width_p(addr_width_p)
          ,.data_width_p(data_width_p)
          ,.x_cord_width_p(x_cord_width_p)
          ,.y_cord_width_p(y_cord_width_p)
        ) hor_e_tieoff (
          .clk_i(clk_i)
          ,.reset_i(reset_r)
          ,.link_sif_i(hor_link_sif_lo[E][j][k])
          ,.link_sif_o(hor_link_sif_li[E][j][k])
        );
      end
    end
  end else begin: cgra_e_tieoff
    // PP: Tie off east HOR links #4 to #7
    for (genvar j = 0; j < num_pods_y_p; j++) begin
      for (genvar k = 4; k < num_tiles_y_p; k++) begin
        bsg_manycore_link_sif_tieoff #(
          .addr_width_p(addr_width_p)
          ,.data_width_p(data_width_p)
          ,.x_cord_width_p(x_cord_width_p)
          ,.y_cord_width_p(y_cord_width_p)
        ) hor_e_tieoff (
          .clk_i(clk_i)
          ,.reset_i(reset_r)
          ,.link_sif_i(hor_link_sif_lo[E][j][k])
          ,.link_sif_o(hor_link_sif_li[E][j][k])
        );
      end
    end
  end


  // RUCHE LINK TIEOFF (west)
  for (genvar j = 0; j < num_pods_y_p; j++) begin
    for (genvar k = 0; k < num_tiles_y_p; k++) begin
      // hard coded for ruche factor 3
      assign ruche_link_li[W][j][k] = '0;
    end
  end

  // RUCHE LINK TIEOFF (east)
  if (bsg_manycore_composition == e_manycore) begin: mc_ruche_e_tieoff
    // PP: This is a pure manycore composition. Tie off east ruche links
    for (genvar j = 0; j < num_pods_y_p; j++) begin
      for (genvar k = 0; k < num_tiles_y_p; k++) begin
        // hard coded for ruche factor 3
        assign ruche_link_li[E][j][k] = '0;
      end
    end
  end else begin: cgra_re_tieoff
    // PP: Tie off east ruche links #4 to #7
    for (genvar j = 0; j < num_pods_y_p; j++) begin
      for (genvar k = 4; k < num_tiles_y_p; k++) begin
        // hard coded for ruche factor 3
        assign ruche_link_li[E][j][k] = '0;
      end
    end
  end

  //-------------------------------------------------------------------------
  // CGRAXcel bay
  //-------------------------------------------------------------------------
  
  if (bsg_manycore_composition == e_manycore_east_cgra_xcel_bay) begin: cgra_xcel_bay
    // In this configuration, we instantiate num_pods_y_p CGRAXcel pods on
    // the east side of the pod array.

    for (genvar i = 0; i < num_pods_y_p; i++) begin: xbay

      // Connections between brg_cgra_pod_sync and pod

      bsg_manycore_link_sif_s [3:0] cs_hor_link_sif_li;
      bsg_manycore_link_sif_s [3:0] cs_hor_link_sif_lo;
      bsg_manycore_ruche_x_link_sif_s [3:0] cs_ruche_link_sif_li;
      bsg_manycore_ruche_x_link_sif_s [3:0] cs_ruche_link_sif_lo;

      for (genvar j = 0; j < 4; j++) begin
        assign cs_hor_link_sif_li[j] = hor_link_sif_lo[E][i][j];
        assign cs_ruche_link_sif_li[j] = ruche_link_lo[E][i][j];
        assign hor_link_sif_li[E][i][j] = cs_hor_link_sif_lo[j];
        assign ruche_link_li[E][i][j] = cs_ruche_link_sif_lo[j];
      end

      brg_cgra_pod_non_synth_sync #(
        .addr_width_p(addr_width_p)
        ,.data_width_p(data_width_p)
        ,.x_cord_width_p(x_cord_width_p)
        ,.y_cord_width_p(y_cord_width_p)
        ,.max_out_credits_p(32)
        ,.pod_y_cord(i)
        ,.ruche_factor_X_p(ruche_factor_X_p)
        ,.num_pods_x_p(num_pods_x_p)
        ,.num_tiles_x_p(num_tiles_x_p)
        ,.num_tiles_y_p(num_tiles_y_p)
        ,.pod_x_cord_width_p(pod_x_cord_width_p)
        ,.pod_y_cord_width_p(pod_y_cord_width_p)
      ) cgra_pod_sync (
        .clk_i                     (clk_i)
        ,.cgra_xcel_clk_i          (cgra_xcel_clk_i)
        ,.reset_i                  (reset_r)
        ,.async_uplink_reset_i     (async_uplink_reset)
        ,.async_downlink_reset_i   (async_downlink_reset)
        ,.async_downstream_reset_i (async_downstream_reset)
        ,.async_token_reset_i      (async_token_reset)

        ,.hor_link_sif_i   (cs_hor_link_sif_li)
        ,.hor_link_sif_o   (cs_hor_link_sif_lo)
        ,.ruche_link_sif_i (cs_ruche_link_sif_li)
        ,.ruche_link_sif_o (cs_ruche_link_sif_lo)
      );

      /* //------------------------------------------------------------- */
      /* // East side SDR interface */
      /* //------------------------------------------------------------- */
      /* // Reference implementation: */
      /* // bigblade_pow_row/v/bsg_manycore_pod_row_sdr.v */

      /* logic [3:0] sdr_e_core_reset_li, sdr_e_core_reset_lo; */
      /* logic [3:0][x_cord_width_p-1:0] sdr_e_core_global_x_li, sdr_e_core_global_x_lo; */
      /* logic [3:0][y_cord_width_p-1:0] sdr_e_core_global_y_li, sdr_e_core_global_y_lo; */

      /* bsg_manycore_link_sif_s [3:0][S:N] sdr_e_ver_link_sif_li, sdr_e_ver_link_sif_lo; */
      /* logic [3:0] sdr_e_async_uplink_reset_li,     sdr_e_async_uplink_reset_lo; */
      /* logic [3:0] sdr_e_async_downlink_reset_li,   sdr_e_async_downlink_reset_lo; */
      /* logic [3:0] sdr_e_async_downstream_reset_li, sdr_e_async_downstream_reset_lo; */
      /* logic [3:0] sdr_e_async_token_reset_li,      sdr_e_async_token_reset_lo; */

      /* // Connections to CGRA */
      /* logic [3:0]                   c_hor_io_fwd_link_clk_li; */
      /* logic [3:0][fwd_width_lp-1:0] c_hor_io_fwd_link_data_li; */
      /* logic [3:0]                   c_hor_io_fwd_link_v_li; */
      /* logic [3:0]                   c_hor_io_fwd_link_token_lo; */

      /* logic [3:0]                   c_hor_io_fwd_link_clk_lo; */
      /* logic [3:0][fwd_width_lp-1:0] c_hor_io_fwd_link_data_lo; */
      /* logic [3:0]                   c_hor_io_fwd_link_v_lo; */
      /* logic [3:0]                   c_hor_io_fwd_link_token_li; */

      /* logic [3:0]                   c_hor_io_rev_link_clk_li; */
      /* logic [3:0][rev_width_lp-1:0] c_hor_io_rev_link_data_li; */
      /* logic [3:0]                   c_hor_io_rev_link_v_li; */
      /* logic [3:0]                   c_hor_io_rev_link_token_lo; */

      /* logic [3:0]                   c_hor_io_rev_link_clk_lo; */
      /* logic [3:0][rev_width_lp-1:0] c_hor_io_rev_link_data_lo; */
      /* logic [3:0]                   c_hor_io_rev_link_v_lo; */
      /* logic [3:0]                   c_hor_io_rev_link_token_li; */

      /* // PP: only connect 4 links to the CGRA half pod */
      /* for (genvar y = 0; y < 4; y++) begin: sdr_e_y */
      /*   bsg_manycore_link_ruche_to_sdr_east #( */
      /*     .lg_fifo_depth_p                  (sdr_lg_fifo_depth_gp) */
      /*     ,.lg_credit_to_token_decimation_p (sdr_lg_credit_to_token_decimation_gp) */

      /*     ,.x_cord_width_p      (x_cord_width_p) */
      /*     ,.y_cord_width_p      (y_cord_width_p) */
      /*     ,.addr_width_p        (addr_width_p) */
      /*     ,.data_width_p        (data_width_p) */
      /*     ,.ruche_factor_X_p    (ruche_factor_X_p) */
      /*   ) sdr_e ( */
      /*     .core_clk_i       (clk_i) */
      /*     ,.core_reset_i    (sdr_e_core_reset_li[y]) */
      /*     ,.core_reset_o    (sdr_e_core_reset_lo[y]) */
      
      /*     ,.core_ver_link_sif_i   (sdr_e_ver_link_sif_li[y]) */
      /*     ,.core_ver_link_sif_o   (sdr_e_ver_link_sif_lo[y]) */

      /*     ,.core_hor_link_sif_i   (hor_link_sif_lo[E][i][y]) */
      /*     ,.core_hor_link_sif_o   (hor_link_sif_li[E][i][y]) */

      /*     ,.core_ruche_link_i     (ruche_link_lo[E][i][y]) */
      /*     ,.core_ruche_link_o     (ruche_link_li[E][i][y]) */

      /*     ,.core_global_x_i       (sdr_e_core_global_x_li[y]) */
      /*     ,.core_global_y_i       (sdr_e_core_global_y_li[y]) */
      /*     ,.core_global_x_o       (sdr_e_core_global_x_lo[y]) */
      /*     ,.core_global_y_o       (sdr_e_core_global_y_lo[y]) */

      /*     ,.async_uplink_reset_i      (sdr_e_async_uplink_reset_li[y]) */
      /*     ,.async_downlink_reset_i    (sdr_e_async_downlink_reset_li[y]) */
      /*     ,.async_downstream_reset_i  (sdr_e_async_downstream_reset_li[y]) */
      /*     ,.async_token_reset_i       (sdr_e_async_token_reset_li[y]) */

      /*     ,.async_uplink_reset_o      (sdr_e_async_uplink_reset_lo[y]) */
      /*     ,.async_downlink_reset_o    (sdr_e_async_downlink_reset_lo[y]) */
      /*     ,.async_downstream_reset_o  (sdr_e_async_downstream_reset_lo[y]) */
      /*     ,.async_token_reset_o       (sdr_e_async_token_reset_lo[y]) */

      /*     ,.io_fwd_link_clk_o       (c_hor_io_fwd_link_clk_li[y]) */
      /*     ,.io_fwd_link_data_o      (c_hor_io_fwd_link_data_li[y]) */
      /*     ,.io_fwd_link_v_o         (c_hor_io_fwd_link_v_li[y]) */
      /*     ,.io_fwd_link_token_i     (c_hor_io_fwd_link_token_lo[y]) */

      /*     ,.io_fwd_link_clk_i       (c_hor_io_fwd_link_clk_lo[y]) */
      /*     ,.io_fwd_link_data_i      (c_hor_io_fwd_link_data_lo[y]) */
      /*     ,.io_fwd_link_v_i         (c_hor_io_fwd_link_v_lo[y]) */
      /*     ,.io_fwd_link_token_o     (c_hor_io_fwd_link_token_li[y]) */

      /*     ,.io_rev_link_clk_o       (c_hor_io_rev_link_clk_li[y]) */
      /*     ,.io_rev_link_data_o      (c_hor_io_rev_link_data_li[y]) */
      /*     ,.io_rev_link_v_o         (c_hor_io_rev_link_v_li[y]) */
      /*     ,.io_rev_link_token_i     (c_hor_io_rev_link_token_lo[y]) */

      /*     ,.io_rev_link_clk_i       (c_hor_io_rev_link_clk_lo[y]) */
      /*     ,.io_rev_link_data_i      (c_hor_io_rev_link_data_lo[y]) */
      /*     ,.io_rev_link_v_i         (c_hor_io_rev_link_v_lo[y]) */
      /*     ,.io_rev_link_token_o     (c_hor_io_rev_link_token_li[y]) */
      /*   ); */

      /*   // connect between sdr east */
      /*   if (y < 3) begin */
      /*     // core reset */
      /*     assign sdr_e_core_reset_li[y+1] = sdr_e_core_reset_lo[y]; */
      /*     // core global cord */
      /*     assign sdr_e_core_global_x_li[y+1] = sdr_e_core_global_x_lo[y]; */
      /*     assign sdr_e_core_global_y_li[y+1] = sdr_e_core_global_y_lo[y]; */
      /*     // ver link */
      /*     assign sdr_e_ver_link_sif_li[y][S] = sdr_e_ver_link_sif_lo[y+1][N]; */
      /*     assign sdr_e_ver_link_sif_li[y+1][N] = sdr_e_ver_link_sif_lo[y][S]; */
      /*     // async reset */
      /*     assign sdr_e_async_uplink_reset_li[y+1] = sdr_e_async_uplink_reset_lo[y]; */
      /*     assign sdr_e_async_downlink_reset_li[y+1] = sdr_e_async_downlink_reset_lo[y]; */
      /*     assign sdr_e_async_downstream_reset_li[y+1] = sdr_e_async_downstream_reset_lo[y]; */
      /*     assign sdr_e_async_token_reset_li[y+1] = sdr_e_async_token_reset_lo[y]; */
      /*   end else begin */
      /*     // core reset */
      /*     assign sdr_e_core_reset_li[0] = reset_r; */
      /*     // core global cord */
      /*     assign sdr_e_core_global_x_li[0] = { (pod_x_cord_width_p)'(1+num_pods_x_p), (x_subcord_width_lp)'(0) }; */
      /*     assign sdr_e_core_global_y_li[0] = { (pod_y_cord_width_p)'(1+2*i), (y_subcord_width_lp)'(0) }; */
      /*     // ver link -- tieoff */
      /*     bsg_manycore_link_sif_tieoff #( */
      /*       .addr_width_p(addr_width_p) */
      /*       ,.data_width_p(data_width_p) */
      /*       ,.x_cord_width_p(x_cord_width_p) */
      /*       ,.y_cord_width_p(y_cord_width_p) */
      /*     ) sdr_ver_n0o_tieoff ( */
      /*       .clk_i(clk_i) */
      /*       ,.reset_i(reset_r) */
      /*       ,.link_sif_i(sdr_e_ver_link_sif_lo[0][N]) */
      /*       ,.link_sif_o(sdr_e_ver_link_sif_li[3][S]) */
      /*     ); */
      /*     bsg_manycore_link_sif_tieoff #( */
      /*       .addr_width_p(addr_width_p) */
      /*       ,.data_width_p(data_width_p) */
      /*       ,.x_cord_width_p(x_cord_width_p) */
      /*       ,.y_cord_width_p(y_cord_width_p) */
      /*     ) sdr_ver_n0i_tieoff ( */
      /*       .clk_i(clk_i) */
      /*       ,.reset_i(reset_r) */
      /*       ,.link_sif_i(sdr_e_ver_link_sif_lo[3][S]) */
      /*       ,.link_sif_o(sdr_e_ver_link_sif_li[0][N]) */
      /*     ); */
      /*     // async reset */
      /*     assign sdr_e_async_uplink_reset_li[0] = async_uplink_reset; */
      /*     assign sdr_e_async_downlink_reset_li[0] = async_downlink_reset; */
      /*     assign sdr_e_async_downstream_reset_li[0] = async_downstream_reset; */
      /*     assign sdr_e_async_token_reset_li[0] = async_token_reset; */
      /*   end */

      /* end  // for: sdr_e_y */

      /* //------------------------------------------------------------- */
      /* // CGRA half pod instantiation */
      /* //------------------------------------------------------------- */

      /* logic [3:0][y_cord_width_p-1:0] c_global_y_cord_li; */
      /* assign c_global_y_cord_li[0] = { (pod_y_cord_width_p)'(1+i), (y_subcord_width_lp)'(0) }; */
      /* assign c_global_y_cord_li[1] = { (pod_y_cord_width_p)'(1+i), (y_subcord_width_lp)'(1) }; */
      /* assign c_global_y_cord_li[2] = { (pod_y_cord_width_p)'(1+i), (y_subcord_width_lp)'(2) }; */
      /* assign c_global_y_cord_li[3] = { (pod_y_cord_width_p)'(1+i), (y_subcord_width_lp)'(3) }; */

      /* brg_cgra_pod #( */
      /*   .addr_width_p(addr_width_p) */
      /*   ,.data_width_p(data_width_p) */
      /*   ,.x_cord_width_p(x_cord_width_p) */
      /*   ,.y_cord_width_p(y_cord_width_p) */
      /*   ,.max_out_credits_p(32) */
      /* ) cgra_bay ( */
      /*   .clk_i(cgra_xcel_clk_i) */
      /*   ,.reset_i(reset_r) */
      /*   ,.global_y_cord_i(c_global_y_cord_li) */

      /*   ,.async_uplink_reset_i(async_uplink_reset) */
      /*   ,.async_downlink_reset_i(async_downlink_reset) */
      /*   ,.async_downstream_reset_i(async_downstream_reset) */
      /*   ,.async_token_reset_i(async_token_reset) */

      /*   ,.async_uplink_reset_o() */
      /*   ,.async_downlink_reset_o() */
      /*   ,.async_downstream_reset_o() */
      /*   ,.async_token_reset_o() */

      /*   ,.io_fwd_link_clk_o(c_hor_io_fwd_link_clk_lo) */
      /*   ,.io_fwd_link_data_o(c_hor_io_fwd_link_data_lo) */
      /*   ,.io_fwd_link_v_o(c_hor_io_fwd_link_v_lo) */
      /*   ,.io_fwd_link_token_i(c_hor_io_fwd_link_token_li) */

      /*   ,.io_fwd_link_clk_i(c_hor_io_fwd_link_clk_li) */
      /*   ,.io_fwd_link_data_i(c_hor_io_fwd_link_data_li) */
      /*   ,.io_fwd_link_v_i(c_hor_io_fwd_link_v_li) */
      /*   ,.io_fwd_link_token_o(c_hor_io_fwd_link_token_lo) */

      /*   ,.io_rev_link_clk_o(c_hor_io_rev_link_clk_lo) */
      /*   ,.io_rev_link_data_o(c_hor_io_rev_link_data_lo) */
      /*   ,.io_rev_link_v_o(c_hor_io_rev_link_v_lo) */
      /*   ,.io_rev_link_token_i(c_hor_io_rev_link_token_li) */

      /*   ,.io_rev_link_clk_i(c_hor_io_rev_link_clk_li) */
      /*   ,.io_rev_link_data_i(c_hor_io_rev_link_data_li) */
      /*   ,.io_rev_link_v_i(c_hor_io_rev_link_v_li) */
      /*   ,.io_rev_link_token_o(c_hor_io_rev_link_token_lo) */
      /* ); */

    end // for: xbay

  end // if: cgra_xcel_bay



//                  //
//    PROFILERS     //
//                  //

if (enable_vcore_profiling_p) begin
  // vanilla core profiler
   bind vanilla_core vanilla_core_profiler #(
    .x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.icache_tag_width_p(icache_tag_width_p)
    ,.icache_entries_p(icache_entries_p)
    ,.data_width_p(data_width_p)
    ,.origin_x_cord_p(`BSG_MACHINE_ORIGIN_X_CORD)
    ,.origin_y_cord_p(`BSG_MACHINE_ORIGIN_Y_CORD)
  ) vcore_prof (
    .*
    ,.global_ctr_i($root.`HOST_MODULE_PATH.global_ctr)
    ,.print_stat_v_i($root.`HOST_MODULE_PATH.print_stat_v)
    ,.print_stat_tag_i($root.`HOST_MODULE_PATH.print_stat_tag)
    ,.trace_en_i($root.`HOST_MODULE_PATH.trace_en)
  );

  bind network_tx remote_load_trace #(
    .addr_width_p(addr_width_p)
    ,.data_width_p(data_width_p)
    ,.x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.pod_x_cord_width_p(pod_x_cord_width_p)
    ,.pod_y_cord_width_p(pod_y_cord_width_p)
    ,.num_tiles_x_p(num_tiles_x_p)
    ,.num_tiles_y_p(num_tiles_y_p)
    ,.origin_x_cord_p(`BSG_MACHINE_ORIGIN_X_CORD)
    ,.origin_y_cord_p(`BSG_MACHINE_ORIGIN_Y_CORD)
  ) rlt (
    .*
    ,.global_ctr_i($root.`HOST_MODULE_PATH.global_ctr)
    ,.trace_en_i($root.`HOST_MODULE_PATH.trace_en)
  );

end

if (enable_cache_profiling_p) begin
  bind bsg_cache vcache_profiler #(
    .data_width_p(data_width_p)
    ,.addr_width_p(addr_width_p)
    ,.header_print_p({`BSG_STRINGIFY(`HOST_MODULE_PATH),".testbench.DUT.py[0].px[0].pod.north_vc_x[0].north_vc_row.vc_y[0].vc_x[0].vc.cache.vcache_prof"})
    ,.ways_p(ways_p)
  ) vcache_prof (
    // everything else
    .*
    // bsg_cache_miss
    ,.chosen_way_n(miss.chosen_way_n)
    // from testbench
    ,.global_ctr_i($root.`HOST_MODULE_PATH.global_ctr)
    ,.print_stat_v_i($root.`HOST_MODULE_PATH.print_stat_v)
    ,.print_stat_tag_i($root.`HOST_MODULE_PATH.print_stat_tag)
    ,.trace_en_i($root.`HOST_MODULE_PATH.trace_en)
  );
end

if (enable_router_profiling_p) begin
  bind bsg_mesh_router router_profiler #(
    .x_cord_width_p(x_cord_width_p)
    ,.y_cord_width_p(y_cord_width_p)
    ,.dims_p(dims_p)
    ,.XY_order_p(XY_order_p)
    ,.origin_x_cord_p(`BSG_MACHINE_ORIGIN_X_CORD)
    ,.origin_y_cord_p(`BSG_MACHINE_ORIGIN_Y_CORD)
  ) rp0 (
    .*
    ,.global_ctr_i($root.`HOST_MODULE_PATH.global_ctr)
    ,.trace_en_i($root.`HOST_MODULE_PATH.trace_en)
    ,.print_stat_v_i($root.`HOST_MODULE_PATH.print_stat_v)
  );
end


endmodule
