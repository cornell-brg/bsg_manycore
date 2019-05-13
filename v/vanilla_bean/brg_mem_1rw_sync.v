// Shady Agwa March 18, 2019.
// Wrapper for SRAM 1024X46.
// 1 read-write port SRAMs.
// Only one read or one write may be done per cycle.

module brg_mem_1rw_sync #(parameter width_p=-1
                          , parameter els_p=-1
                          , parameter addr_width_lp=`BSG_SAFE_CLOG2(els_p))
   (input   clk_i
    , input reset_i
    , input [width_p-1:0] data_i
    , input [addr_width_lp-1:0] addr_i
    , input v_i
    , input w_i
    , output logic [width_p-1:0]  data_o
    );
  if ( ( width_p == 46 ) && ( addr_width_lp == 10 ) && ( els_p == 1024 ) )
    brg_sram_1024x46
      #(.width_p(width_p)
       ,.els_p(els_p)
       ) synth
       (.*);
  else if ( ( width_p == 8 ) && ( addr_width_lp == 10 ) && ( els_p == 1024 ) )
    brg_sram_1024x8
      #(.width_p(width_p)
       ,.els_p(els_p)
       ) synth
       (.*);

  else
     bsg_mem_1rw_sync_synth
       #(.width_p(width_p)
       ,.els_p(els_p)
       ) synth
       (.clk_i(clk_i), .reset_i(reset_i), .w_i(w_i), .addr_i(addr_i), .data_i(data_i),
        .v_i(v_i), .data_o(data_o));

endmodule
