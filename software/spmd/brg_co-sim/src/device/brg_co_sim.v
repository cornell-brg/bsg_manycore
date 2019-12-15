// `include "bsg_manycore_packet.vh"
// `include "bsg_manycore_addr.vh"
`include "brg_cosim_packet.v"

module brg_co_sim #(
  // parameter a = 1
) 
(
  input logic         clk, 
  input logic         reset,

  // output logic  [1:0] req,
  // output logic        req_val,
  // input logic         req_rdy,

  input logic  [31:0] in,
  input logic         in_val,
  output logic        in_rdy,
  
  output logic [31:0] out,
  output logic [31:0] out_addr,
  output logic        out_val,
  input  logic        out_rdy
  
  // output logic [31:0] out_size
);
/**
  To facilitate communication between the host and manycore processor,
  we implement control bits that can freeze tiles and unfreeze tiles when
  they are waiting for requests from the host.
*/

  integer st_free, st_curr;
  typedef struct packed {
    logic valid;
    logic [31:0] data;    //entry is valid
    logic [31:0] addr; 
  } payload;
  payload storage [1000:0];
 //----------------------------------------------------------------------
  // sverilog functions and C functions
  //---------------------------------------------------------------------

  export "DPI" function write_mem;
  function void write_mem(int addr, int data);
    storage[st_free].data = data;
    storage[st_free].addr = addr;
    storage[st_free].valid = 1;
    st_free = st_free + 1;
  endfunction // write_mem

  export "DPI" function read_mem;
  function int read_mem(int addr);
    return -1;
  endfunction // read_mem

  import "DPI" context function void shm_init();
  import "DPI" context function int receive_request();
  integer r;
  
  initial begin
    $display("___________ %L: shm_init() ___________");
    shm_init();

  end

  // One cycle latency
  // ctrl signals
  always_ff @(posedge clk or posedge reset) begin
    if ( reset ) begin
      st_free  <= 0;
      st_curr  <= 0;
    end
    else begin
      if (st_curr < st_free) begin
        st_curr <= st_curr + 1;
        storage[st_curr].valid <= 0;
      end
      else begin
        st_curr <= 0;
        st_free <= 0;
      end
    end
  end

  always_comb begin
    if ( reset ) begin
      out      = 32'bx;
      out_addr = 32'bx;
      out_val  = 0;
    end
    else begin
      if ( storage[st_curr].valid ) begin
        out      = storage[st_curr].data;
        out_addr = storage[st_curr].addr;
        out_val  = storage[st_curr].valid;
      end
    end
  end

  always_ff @(posedge clk) begin
    if (reset)     r = 1;
    else if (r == 1) begin
      r = receive_request();
      $display("___________ %L: received request %d ___________", r);
    end
  end

  // task write();
    
  //   $display("Writing ____________________________");
  // endtask


endmodule