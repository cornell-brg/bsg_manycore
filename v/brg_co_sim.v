`include "bsg_manycore_packet.vh"
// `include "bsg_manycore_addr.vh"
`include "brg_cosim_packet.vh"

`define UNFREEZE_ADDR 32'h00020000
`define BASE_ADDR 32'h0000fff0

module brg_co_sim #(
  parameter on = 0
) 
(
  input logic         clk, 
  input logic         reset,

  input logic  [31:0] in,
  input logic         in_val,
  output logic [31:0] in_addr,
  output logic        in_rdy,
  
  output logic [31:0] out,
  output logic [31:0] out_addr,
  output logic        out_val,
  input  logic        out_rdy,
  

  output control      ctrl_out,
  output logic        ctrl_out_val,

  input  control      ctrl_in,
  input  logic        ctrl_in_val
);
/**
  To facilitate communication between the host and manycore processor,
  we implement control bits that can freeze tiles and unfreeze tiles when
  they are waiting for requests from the host.
*/
// st_free : pointer to free space in storage for
// one memcpy. Resets per memcpy
// st_curr : pointer to current element in storage to be
// written to DRAM. Resets per memcpy
  integer st_free, st_curr;
  typedef struct packed {
    logic valid;
    logic [31:0] data;    //entry is valid
    logic [31:0] addr; 
  } payload;
  payload storage [100:0];
 //----------------------------------------------------------------------
  // sverilog functions and C functions
  //---------------------------------------------------------------------

  parameter STATE_INIT   = 0;
  parameter STATE_WAIT   = 1; // wait for input from host
  parameter STATE_MEMCPY = 2;
  parameter STATE_GO     = 3; // start co-simulation
  parameter STATE_MALLOC = 4;
  parameter STATE_STOP   = 5;
  parameter STATE_GO_WAIT= 6; // Wait for co-sim to end
  parameter STATE_FEEDBACK= 7; // Report back to host that sim has ended
  parameter STATE_DEV_HOST= 8; // Memcpy device to host

  logic [3:0] curr_state;
  logic [3:0] next_state;

  control ctrl_bits;
  logic [31:0] dev_ptr;

  int mem_req_size, mem_req_ptr, mem_req_stop;
  export "DPI" function write_mem;
  function void write_mem(int addr, int data);
    storage[st_free].data = data;
    storage[st_free].addr = addr;
    storage[st_free].valid = 1;
    st_free = st_free + 1;
  endfunction // write_mem

  export "DPI" function read_mem_req;
  function void read_mem_req(int addr, int size);
    mem_req_ptr = addr;
    mem_req_size = size;
    mem_req_stop = (mem_req_size-1)*4 + mem_req_ptr;
  endfunction // read_mem
  
  export "DPI" function cosim_malloc;
  function int cosim_malloc(int size);
    int tmp_ptr; 
    tmp_ptr = dev_ptr;
    dev_ptr = dev_ptr + size*4;
    return tmp_ptr;
  endfunction 

  import "DPI" context function void shm_init();
  import "DPI" context function int receive_request();
  import "DPI" context function int send_request(int data);
  import "DPI" context function void build_response(int data);
  import "DPI" context function int send_response();
  
  logic go,valid,done;
  // assign go = ctrl_bits.cosim_go;
  // assign valid = ctrl_bits.cosim_val;
  // assign done = ctrl_bits.cosim_done;

  integer r;
  
  initial begin
    $display("___________ %L: shm_init() ___________");
    shm_init();
    valid = 0;
  end

  always_ff @(posedge clk) begin
    if (reset)
      curr_state <= STATE_INIT;
    else
      curr_state <= next_state;
    // if (curr_state != STATE_STOP) begin
    //   $display("cur_state = %d", curr_state);
    //   $display("next_state = %d", next_state);
    // end
    // if (curr_state != next_state) begin
    //   $display("STATE:%d",curr_state);
    // end
  end

  always_comb begin
    case (curr_state)
      STATE_INIT: begin
        if (valid) next_state = STATE_WAIT;
        else next_state = STATE_INIT;
        // next_state = STATE_INIT;
      end
      STATE_WAIT: begin
        if ( r == STATE_GO ) next_state = STATE_GO;
        else if (r == STATE_MEMCPY) next_state = STATE_MEMCPY;
        else if (r == STATE_MALLOC) next_state = STATE_MALLOC;
        else if (r == STATE_STOP)   next_state = STATE_STOP;
        else if (r == STATE_DEV_HOST) next_state = STATE_DEV_HOST;
        else next_state = STATE_WAIT;
      end
      STATE_GO: begin
        next_state = STATE_GO_WAIT;
      end
      STATE_GO_WAIT: begin
        if ( done ) next_state = STATE_FEEDBACK;
        else next_state = STATE_GO_WAIT;
      end
      STATE_FEEDBACK: begin
        next_state = STATE_INIT;
      end
      STATE_MALLOC: begin
        next_state = STATE_WAIT;
      end
      STATE_MEMCPY: begin
        if ( st_curr >= st_free ) next_state = STATE_WAIT; 
        else next_state = STATE_MEMCPY;
      end
      STATE_DEV_HOST: begin
        if (  mem_req_ptr < mem_req_stop ) next_state = STATE_DEV_HOST;
        else next_state = STATE_WAIT;
      end
      STATE_STOP: begin
        next_state = STATE_STOP;
      end
      default: next_state = STATE_STOP;
    endcase
  end

  always @(*) begin
    case (curr_state) 
      STATE_INIT: begin
        $display("%L: STATE_INIT");
        out_val = 0;
        ctrl_out_val = 0;
        dev_ptr = `BASE_ADDR;
        in_rdy = 0;
        in_addr =32'hffff;
        if (ctrl_in_val) begin
          valid = ctrl_in.cosim_val;  
        end
      end
      STATE_WAIT: begin
        // $display("%L: STATE_WAIT");
      // wait for the host to tell the processor to go.   
        out_val = 0;
        // out_addr = `UNFREEZE_ADDR;
        // out = 1;
        ctrl_out_val = 0;
        in_rdy  = 0;

      end
      STATE_GO: begin
      // Processor is going; need to poll for when proc is done.
        out_val = 0;
        ctrl_out_val = 1;
        ctrl_out.cosim_val  = 1;
        ctrl_out.cosim_stop = 0;
        ctrl_out.cosim_go   = 1;
        ctrl_out.cosim_done = 0;
       
      end
      STATE_GO_WAIT: begin 
        out_val = 0;
        ctrl_out_val = 0; 
        if (ctrl_in_val) 
          done = ctrl_in.cosim_done;
      end
      STATE_FEEDBACK: begin
        send_request( done );
      end
      STATE_MALLOC: begin
        // $display("%L: STATE_MALLOC");
        // out_val = 1;
        // out_addr = `UNFREEZE_ADDR;
        // out = 0;
        out_val = 0;
        ctrl_out_val = 0;
      end
      STATE_MEMCPY: begin
        // $display("%L: STATE_MEMCPY");
        // $display("free = %d, curr = %d", st_free,st_curr);
        ctrl_out_val = 0;
        if ( storage[st_curr].valid ) begin
          out      = storage[st_curr].data;
          out_addr = storage[st_curr].addr;
          out_val  = storage[st_curr].valid;
        end
      end
      STATE_DEV_HOST: begin
        // $display("%L: STATE_DEV_HOST");
        out_val = 0;
        ctrl_out_val = 0;
        in_rdy = 1;
        in_addr = mem_req_ptr;

      end
      STATE_STOP: begin
        $display("%L: STATE_STOP");
        out_val = 0;
        ctrl_out_val = 1;
        ctrl_out.cosim_val  = 1;
        ctrl_out.cosim_stop = 1;
        ctrl_out.cosim_go   = 0;
        ctrl_out.cosim_done = 0;
      end
      default: begin
        $display("STATE_UD");
        out_val = 0;
        ctrl_out_val = 0;
      end
    endcase
  end

  // One cycle latency
  // ctrl signals
  always_ff @(posedge clk or posedge reset) begin
    if ( reset ) begin
      st_free  <= 0;
      st_curr  <= 0;
    end
    else begin
      if (curr_state == STATE_MEMCPY && st_curr < st_free) begin
        st_curr <= st_curr + 1;
        storage[st_curr].valid <= 0;  
      end
      else if (curr_state == STATE_MEMCPY && next_state == STATE_WAIT) begin
        st_free <= 0;
        st_curr <= 0;
      end
      else begin
        st_curr <= 0;
      end

      if (curr_state == STATE_WAIT && next_state == STATE_WAIT) begin
        $display("____________%L: Waiting for request____________");
        r = receive_request();
        $display("___________ %L: received request %d ___________", r);
      end
      else begin
        r = STATE_WAIT;
      end
      if (curr_state == STATE_DEV_HOST) begin
        if (in_val) begin
          build_response (in);
        end
        mem_req_ptr <= mem_req_ptr + 4;
      end
      if (curr_state == STATE_DEV_HOST && next_state == STATE_WAIT) begin
        send_response();
      end
    end
  end
  

  // always_ff @(posedge clk) begin
  //   if (ctrl_out_val == 0)
  //     ctrl_out_val <= ctrl_val;
  //   else
  //     ctrl_out_val <= 0;

  //   if (ctrl_in_val) 
  //     send_request(ctrl_in);
  // end

  // always_comb begin
  //   if ( reset ) begin
  //     out      = 32'bx;
  //     out_addr = 32'bx;
  //     out_val  = 0;
  //   end
  //   else begin
  //     if ( storage[st_curr].valid ) begin
  //       out      = storage[st_curr].data;
  //       out_addr = storage[st_curr].addr;
  //       out_val  = storage[st_curr].valid;
  //     end
  //   end
  // end

  // always_ff @(posedge clk) begin
  //   if (reset)     r = 1;
  //   else if (r == 1 && on) begin
  //     r = receive_request();
  //     $display("___________ %L: received request %d ___________", r);
  //   end
  // end


endmodule