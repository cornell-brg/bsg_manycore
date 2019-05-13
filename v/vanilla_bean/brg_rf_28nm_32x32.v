// Shady Agwa March 2019.
// Macro Box for Register File 32x32.

module rf_28nm_32x32_SP (cenya, aya, cenyb, wenyb, ayb, qa, soa, sob, clka, cena, aa, clkb, cenb, wenb, ab, db, emaa, emasa, emab, tena, tcena, taa, tenb, tcenb, twenb, tab, tdb, ret1n, sia, sea, dftrambyp, sib, seb, colldisn);

  output  cenya;
  output [4:0] aya;
  output  cenyb;
  output [31:0] wenyb;
  output [4:0] ayb;
  output [31:0] qa;
  output [1:0] soa;
  output [1:0] sob;
  input  clka;
  input  cena;
  input [4:0] aa;
  input  clkb;
  input  cenb;
  input [31:0] wenb;
  input [4:0] ab;
  input [31:0] db;
  input [2:0] emaa;
  input  emasa;
  input [2:0] emab;
  input  tena;
  input  tcena;
  input [4:0] taa;
  input  tenb;
  input  tcenb;
  input [31:0] twenb;
  input [4:0] tab;
  input [31:0] tdb;
  input  ret1n;
  input [1:0] sia;
  input  sea;
  input  dftrambyp;
  input [1:0] sib;
  input  seb;
  input  colldisn;
  // synopsys translate_off
  bsg_mem_1r1w_sync_synth #(.width_p(32),.els_p(32)) 
                          synth 
                           ( .clk_i(clka), .reset_i(1'b0), .w_v_i(cenb), .w_addr_i(ab), .w_data_i(db)
                           , .r_v_i(cena), .r_addr_i(aa), .r_data_o(qa)
                           );
  // synopsys translate_on
endmodule 
