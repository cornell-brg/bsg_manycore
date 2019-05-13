// Shady Agwa March 19, 2019.
// Wrapper for the Macro Box of 28nm SRAM 1024x46.
// Synchronous 1-port ram.
// Only one read or one write may be done per cycle.

module brg_sram_1024x46 #(parameter width_p=46
                                , parameter els_p=1024
                                , parameter addr_width_lp=10)
( 
  input   clk_i, 
  input v_i,
  input reset_i,
  input [width_p-1:0] data_i,
  input [addr_width_lp-1:0] addr_i,
  input w_i,
  output logic [width_p-1:0]  data_o
);
  // Declare wiring Interface 
  wire ceny;
  wire [45:0] weny;
  wire [9:0] ay;
  wire gweny;
  wire [1:0] so;
  wire brg_ceny;
  wire [45:0] brg_weny;
  wire [9:0] brg_ay;
  wire brg_gweny;
  wire [45:0] q;
  wire [1:0] brg_so;
  wire  clk;
  wire  cen;
  wire [45:0] wen;
  wire [9:0] a;
  wire [45:0] d;
  wire [2:0] ema;
  wire [1:0] emaw;
  wire  emas;
  wire  ten;
  wire  tcen;
  wire [45:0] twen;
  wire [9:0] ta;
  wire [45:0] td;
  wire  gwen;
  wire  tgwen;
  wire  ret1n;
  wire [1:0] si;
  wire  se;
  wire  dftrambyp;
  wire unused = reset_i;
  //Mapp the required  ports to the SRAM
  assign clk = clk_i;
  assign a = addr_i;
  assign d = data_i;
  assign data_o = q;
  assign gwen = ~ w_i;
  assign cen = ~ v_i;
  // Assign parameters of ARM SRAM
  assign dftrambyp = 1'd0;
  assign ema = 3'd3;
  assign emas = 1'd0;
  assign emaw = 2'd1;
  assign ret1n = 1'd1;
  assign se = 1'd0;
  assign si = 2'd0;
  assign ta = 10'd0;
  assign tcen = 1'd0;
  assign td = 46'd0;
  assign ten = 1'd1;
  assign tgwen = 1'd0;
  assign twen = 46'd0;
  assign wen = {46{gwen}};
  // Unused output from SRAM
  assign brg_ceny = ceny;
  assign brg_weny = weny;
  assign brg_ay = ay;
  assign brg_gweny = gweny;
  assign brg_so = so;
 // .emas(emas), for sram_sp
  sram_28nm_1024x46_SP sram_1024(.ceny(ceny), .weny(weny), .ay(ay), .gweny(gweny), .q(q),
                                   .so(so), .clk(clk), .cen(cen), .wen(wen), .a(a), .d(d), .ema(ema),
                                   .emaw(emaw), .ten(ten), .tcen(tcen), .twen(twen), .ta(ta),
                                   .td(td), .gwen(gwen), .tgwen(tgwen), .ret1n(ret1n), .si(si), .se(se), .dftrambyp(dftrambyp));
endmodule 
