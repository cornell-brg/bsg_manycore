//  Shady Agwa June 10, 2019.
// non-synthesized code for CSR Stats_en.
// stats_en is changed @ wb stage 
// it is used to trace toggling for SAIF 

`include "parameters.vh"
`include "definitions.vh"

module brg_csr(
		input                             clk_i
               ,input                             reset_i
	       ,input [31:0]		  instruction
	       ,output reg stats_en
);
  logic stats_brg;
  always @(posedge clk_i)
  begin
    if (reset_i)
      stats_brg <= 0;
    else 
    begin
      // check for the op code & register id 0x7c1 & function Write immediate
      // @ execution stage
      if ((instruction[6-:7] == `RV32_SYSTEM) &&  (instruction[31-:12] == 12'h7c1) && (instruction[14-:3] == `RV32_CSRRWI_FUN3)) 
        begin
          // Write the least sig. bit of imm to stats register @ mem stage 
          stats_brg <= instruction[15]; //19 -5 
        end
    end
  end
    // write the stats_en register @ wb stage 
    // delay the trigger action till the instrcution commits
  always @(posedge clk_i)
    if (reset_i)
      stats_en <= 0;
    else
      stats_en <= stats_brg;
endmodule 
