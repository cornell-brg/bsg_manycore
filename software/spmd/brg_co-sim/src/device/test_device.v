//========================================================================
// test_device.v
//========================================================================

`timescale 1ns/10ps

//------------------------------------------------------------------------
// test memory
//------------------------------------------------------------------------
// synchronous test memory

module test_sync_memory
(
  input  logic         clk,
  input  logic         reset,

  // Read port (synchronous read)

  input  logic         read_en,
  input  logic [9:0]   read_addr,
  output logic [31:0]  read_data,

  // Write port (sampled on the rising clock edge)

  input  logic         write_en,
  input  logic [9:0]   write_addr,
  input  logic [31:0]  write_data
);

  logic [31:0] mem[1023:0];

  // Combinational read. We ensure the read data is all X's if we are
  // doing a write because we are modeling an SRAM with a single
  // read/write port (i.e., not a dual ported SRAM). We also ensure the
  // read data is all X's if the read is not enable at all to avoid
  // (potentially) incorrectly assuming the SRAM latches the read data.

  always @( posedge clk ) begin
    if ( read_en )
      read_data <= mem[read_addr];
    else
      read_data <= 'hx;
  end


  always @( posedge clk ) begin
    if ( write_en )
      mem[write_addr] <= write_data;
  end

endmodule // test_sync_memory

//------------------------------------------------------------------------
// tb
//------------------------------------------------------------------------

module test_device_tb ();

  // clock (not used)
  logic clk = 1'b1;
  always #5 clk = ~clk;

  // other not used signal
  logic        reset;
  logic        read_en;
  logic [9:0]  read_addr;
  logic [31:0] read_data;
  logic        write_en;
  logic [9:0]  write_addr;
  logic [31:0] write_data;

  test_sync_memory test_mem(.clk(clk), .reset(reset), .read_en(read_en),
                            .read_addr(read_addr), .read_data(read_data),
                            .write_en(write_en), .write_addr(write_addr),
                            .write_data(write_data)
                            );

  //----------------------------------------------------------------------
  // sverilog functions and C functions
  //----------------------------------------------------------------------

  export "DPI" function write_mem;
  function void write_mem(int addr, int data);
    test_mem.mem[addr >> 2] = data;
  endfunction // write_mem

  export "DPI" function read_mem;
  function int read_mem(int addr);
    return test_mem.mem[addr];
  endfunction // read_mem

  import "DPI" context function void shm_init();
  import "DPI" context function int receive_request();

  //----------------------------------------------------------------------
  // initial block
  //----------------------------------------------------------------------

  int r;

  initial begin
    // initialize shm
    shm_init();

    // loop untill receive stop signal
    while (1) begin
      r = receive_request();
      if (r == 0) begin
        $display("Received stop signal");
        break;
      end
    end

    // display content of the memory
    $display("memory content:");

    for (int i = 0; i < 8; i++) begin
      $display("memory[%d] = %d", i, test_mem.mem[i]);
    end
  end

endmodule
