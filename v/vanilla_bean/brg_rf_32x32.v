// Shady Agwa March 18, 2019.
// Wrapper for the Macro Box of the Register File 32x32.
// Signals are mapped to the targated 28nm Memory Compiler RF.

module brg_rf32x32 #(parameter width_p= 32
                                 , parameter els_p= 32
                                 , parameter addr_width_lp=5
                                 )
   (input   clk_i
    , input reset_i

    , input                     w_v_i
    , input [addr_width_lp-1:0] w_addr_i
    , input [width_p-1:0]       data_i
    , input                     v_i
    , input [addr_width_lp-1:0] addr_i
    , output logic [width_p-1:0] data_o
);

  wire  cenya;
  wire [4:0] aya;
  wire  cenyb;
  wire [31:0] wenyb;
  wire [4:0] ayb;
  wire [31:0] qa;
  wire [1:0] soa;
  wire [1:0] sob;


  wire  cena;
  wire [4:0] aa;

  wire  cenb;
  wire [31:0] wenb;
  wire [4:0] ab;
  wire [31:0] db;

  wire [2:0] emaa;
  wire  emasa;
  wire [2:0] emab;
  wire  tena;
  wire  tcena;
  wire [4:0] taa;
  wire  tenb;
  wire  tcenb;
  wire [31:0] twenb;
  wire [4:0] tab;
  wire [31:0] tdb;
  wire  ret1n;
  wire [1:0] sia;
  wire  sea;
  wire  dftrambyp;
  wire [1:0] sib;
  wire  seb;
  wire  colldisn;

  assign cenb  = w_v_i;
  assign ab = w_addr_i;
  assign db = data_i;
  assign cena = v_i;  
  assign aa = addr_i;
  assign data_o = qa;
  assign emaa = 3'd3;
  assign emasa = 1'd0;
  assign emab = 3'd3;
  assign tena = 1'd1;
  assign tcena = 1'd0;
  assign taa = 5'd0;
  assign tenb = 1'd1;
  assign tcenb = 1'd0;
  assign tab = 5'd0;
  assign twenb = 32'd0;
  assign tdb = 32'd0;
  assign ret1n = 1'd1;
  assign sea = 1'd0;
  assign sia = 2'd0;
  assign seb = 1'd0;
  assign sib = 2'd0;
  assign dftrambyp = 1'd0;
  assign colldisn = 1'd0;

  rf_28nm_32x32_SP rf_32x32(.cenya(cenya), .aya(aya), .cenyb(cenyb), .wenyb(wenyb), .ayb(ayb), .qa(qa), .soa(soa), .sob(sob), .clka(clk_i), .cena(cena), .aa(aa), .clkb(clk_i), .cenb(cenb), .wenb(wenb), .ab(ab), .db(db), .emaa(emaa), .emasa(emasa), .emab(emab), .tena(tena), .tcena(tcena), .taa(taa), .tenb(tenb), .tcenb(tcenb), .twenb(twenb), .tab(tab), .tdb(tdb), .ret1n(ret1n), .sia(sia), .sea(sea), .dftrambyp(dftrambyp), .sib(sib), .seb(seb), .colldisn(colldisn));


endmodule


