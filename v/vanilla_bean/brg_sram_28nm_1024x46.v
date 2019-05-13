// Shady Agwa March 19, 2019.
// BlackBox for  28nm SRAM 1024x46.
// Synchronous 1-port ram.
// Only one read or one write may be done per cycle.

module sram_28nm_1024x46_SP (ceny, weny, ay, gweny, q, so, clk, cen, wen, a, d, ema,
    emaw, ten, tcen, twen, ta, td, gwen, tgwen, ret1n, si, se, dftrambyp);
  output  ceny;
  output [45:0] weny;
  output [9:0] ay;
  output  gweny;
  output [45:0] q;
  output [1:0] so;
  input  clk;
  input  cen;
  input [45:0] wen;
  input [9:0] a;
  input [45:0] d;
  input [2:0] ema;
  input [1:0] emaw;
//  input  emas;
  input  ten;
  input  tcen;
  input [45:0] twen;
  input [9:0] ta;
  input [45:0] td;
  input  gwen;
  input  tgwen;
  input  ret1n;
  input [1:0] si;
  input  se;
  input  dftrambyp;
// synopsys translate_off
  wire w_i;
  wire v_i;
  wire clk_i;
  wire [9:0] addr_i;
  wire [45:0] data_i;
  wire [45:0] data_o;

  // Mapp the required  ports to the SRAM
  assign clk_i = clk;
  assign addr_i = a;
  assign data_i = d;
  assign q = data_o;
  assign w_i = ~gwen;
  assign v_i = ~cen;
  bsg_mem_1rw_sync_synth
       #(.width_p(46)
       ,.els_p(1024)
       ) mem
       (.clk_i(clk_i), .reset_i(1'b0), .w_i(w_i), .addr_i(addr_i), .data_i(data_i),
        .v_i(v_i), .data_o(data_o));
// synopsys translate_on
endmodule 
