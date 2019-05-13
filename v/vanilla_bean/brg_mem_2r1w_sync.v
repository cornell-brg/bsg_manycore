// Shady Agwa March 18, 2019.
// Wrapper for Register File 32x32.
// 2 read-port, 1 write-port RF.
// reads are synchronous.

module brg_mem_2r1w_sync #(parameter width_p=-1
                           , parameter els_p=-1
                           , parameter read_write_same_addr_p=0
                           , parameter addr_width_lp=`BSG_SAFE_CLOG2(els_p)
                           , parameter harden_p=0
                           )
   (input   clk_i
    , input reset_i

    , input                     w_v_i
    , input [addr_width_lp-1:0] w_addr_i
    , input [width_p-1:0]       w_data_i

    // currently unused
    , input                      r0_v_i
    , input [addr_width_lp-1:0]  r0_addr_i
    , output logic [width_p-1:0] r0_data_o

    , input                      r1_v_i
    , input [addr_width_lp-1:0]  r1_addr_i
    , output logic [width_p-1:0] r1_data_o
    );
  if ( ( width_p == 31) && ( addr_width_lp == 5 ) && ( els_p == 32 ) ) begin
    brg_rf32x32  #(.width_p(width_p)
                   ,.els_p(els_p)
                  ) rf_0
                  ( .clk_i(clk_i), .reset_i(reset_i), .w_v_i(w_v_i), .w_addr_i(w_addr_i), 
                    .data_i(w_data_i), .v_i(r0_v_i), .addr_i(r0_addr_i), .data_o(r0_data_o));
  
    brg_rf32x32  #(.width_p(width_p)
                   ,.els_p(els_p)
                  ) rf_1
                  ( .clk_i(clk_i), .reset_i(reset_i), .w_v_i(w_v_i), .w_addr_i(w_addr_i), 
                    .data_i(w_data_i), .v_i(r1_v_i), .addr_i(r1_addr_i), .data_o(r1_data_o));
  end
  else begin
  bsg_mem_2r1w_sync_synth
     #(.width_p(width_p)
       ,.els_p(els_p)
       ,.read_write_same_addr_p(read_write_same_addr_p)
       ,.harden_p(harden_p)
       ) synth
       (.*);
  end 
endmodule
