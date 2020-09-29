module BrgHBFreezeUnit
  import bsg_manycore_pkg::*;
#(
   x_cord_width_p                 = "inv"
  ,y_cord_width_p                 = "inv"
  ,data_width_p                   = 32
  ,addr_width_p                   = 32
  ,return_packet_width_lp         = `bsg_manycore_return_packet_width(x_cord_width_p,y_cord_width_p,data_width_p)
)
(
   input                    clk_i
  ,input                    reset_i
  ,input                    in_v_i
  ,input [addr_width_p-1:0] in_addr_i
  ,output                   in_yumi_o
  ,output                   returning_v_o
  ,output                   freeze_o
);

logic freeze_r;
logic freeze_prev;
logic returning_v_o;

assign freeze_o = freeze_r;
assign in_yumi_o = in_v_i;

always_comb begin
  returning_v_o = 1'b0;
  if (freeze_r == 1'b0 & freeze_prev == 1'b1) begin
    returning_v_o = 1'b1;
  end
end

always_ff @(posedge clk_i) begin
  if (reset_i) begin
    freeze_r    <= 1'b1;
    freeze_prev <= 1'b1;
  end
  else begin
    freeze_prev <= freeze_r;
    if (in_v_i & in_addr_i == 'h8000) begin
      freeze_r <= 1'b0;
    end
  end
end

endmodule