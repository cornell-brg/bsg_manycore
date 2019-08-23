
//====================================================================
// brg_xcel_profiler.v
// 08/15/2019, Xiaoyu Yan (xy97@cornell.edu)
//====================================================================
// Keeps track of xcel stats such as cycle count.

module brg_xcel_profiler
  #(
      parameter x_cord_width_p="inv"
    , parameter y_cord_width_p="inv" 
    , parameter count_en_addr="inv" 
    , parameter addr_width_p="inv" 
  )
  (
    input   clk_i
    ,input  reset_i
    ,input  v_i
    ,input  DRAM_req_v_i
    ,input  DRAM_resp_v_i
    ,input [addr_width_p-1:0] addr_i
    ,input  data_i
    ,input [x_cord_width_p-1:0] my_x_i
    ,input [y_cord_width_p-1:0] my_y_i
  );
  string rpt_dir;
  initial begin
    if(!$value$plusargs("rpt_dir=%s", rpt_dir)) //(xy97)
      rpt_dir = "rtl_rpt";
  end
  // Counts for bandwidth utilization
  // Out going signal count: req send to DRAM
  logic [31:0] v_o_count;
  // Incoming signal count: resp from DRAM
  logic [31:0] v_i_count;
  // Counts the cycle 
  logic [31:0] cyc_count;
  logic        cyc_count_en;
  logic [1:0]  curr_state;
  logic [1:0]  next_state;

  localparam STATE_IDLE=0;
  localparam STATE_COUNT=1;
  localparam STATE_LOG=2;

  // State transition
  always_ff @(posedge clk_i) begin
    if (reset_i) curr_state = STATE_IDLE;
    else curr_state = next_state; 
  end

  // Next state logic
  always_comb begin
    next_state = next_state;
    case (curr_state) 
      STATE_IDLE: begin
        if (v_i && addr_i==count_en_addr && data_i) next_state = STATE_COUNT;
      end
      STATE_COUNT: begin
        if (v_i && addr_i==count_en_addr && !data_i) next_state = STATE_LOG;
      end
      STATE_LOG: begin
        next_state = STATE_IDLE;
      end
      default: next_state = STATE_IDLE;
    endcase
  end
  // Output logic
  always_comb begin
    case (curr_state) 
      STATE_IDLE: begin
        cyc_count_en = 0;
      end
      STATE_COUNT: begin
        cyc_count_en = 1;
      end
      STATE_LOG: begin
        cyc_count_en = 0;
      end
      default: cyc_count_en = 0;
    endcase
  end

  //trancing of state
  // always @(next_state||curr_state) begin
  //   $display("STATE: %d", curr_state);
  // end


  // Cycle counter logic
  string fname;
  int f;
  always @(negedge clk_i) begin
    if (reset_i || curr_state == STATE_IDLE) begin
      cyc_count = 0;
      v_o_count = 0;
      v_i_count = 0;
    end
    else if (cyc_count_en) begin
      cyc_count = cyc_count + 1;
      if (DRAM_req_v_i) v_o_count = v_o_count + 1;
      if (DRAM_resp_v_i) v_i_count = v_i_count + 1;
    end
    else begin
      cyc_count = cyc_count;
      v_o_count = v_o_count;
      v_i_count = v_i_count;
    end
    if (curr_state == STATE_LOG) begin
      fname = {$sformatf("%s",rpt_dir),"/",$sformatf("%h",my_x_i),
        "_",$sformatf("%h",my_y_i), "_xcel_stats.json"};
      f = $fopen(fname,"w");
      $fwrite(f,"{\n");
      $fwrite(f,"\"coord\": \"%d %d\",\n", my_x_i, my_y_i);
      $fwrite(f,"\"DRAM req cycles\": %d,\n",v_o_count);
      $fwrite(f,"\"DRAM resp cycles\": %d,\n",v_i_count);
      $fwrite(f,"\"kernel cycles\": %d\n",cyc_count);
      $fwrite(f,"}\n");
      $fclose(f); 
    end    
  end

endmodule