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
   input                       clk_i
  ,input                       reset_i
  ,input  [x_cord_width_p-1:0] my_x_i
  ,input  [y_cord_width_p-1:0] my_y_i
  // in request
  ,input                       in_v_i
  ,input [addr_width_p-1:0]    in_addr_i
  ,input [data_width_p-1:0]    in_data_i
  ,output                      in_yumi_o
  // out response
  ,output                      returning_v_o
  // status output
  ,output                      freeze_o
);

logic freeze_r;
logic freeze_prev;
logic freeze_next;
logic returning_v_o;

assign freeze_o = freeze_r;
assign in_yumi_o = in_v_i;

always_comb begin
  returning_v_o = 1'b0;
  if (freeze_r != freeze_prev) begin
    returning_v_o = 1'b1;
  end
end

assign freeze_next = (in_v_i & in_addr_i == 'h8000) ? in_data_i[0] : freeze_r;

always_ff @(posedge clk_i) begin
  if (reset_i) begin
    freeze_r    <= 1'b1;
    freeze_prev <= 1'b1;
  end
  else begin
    freeze_prev <= freeze_r;
    freeze_r <= freeze_next;
  end
end

// synopsys translate_off

always_ff @ (negedge clk_i) begin
  if (~reset_i) begin
    if (freeze_next & ~freeze_r)
      $display("[INFO][RX] Freezing tile t=%0t, x=%d, y=%d", $time, my_x_i, my_y_i);
    if (~freeze_next & freeze_r)
      $display("[INFO][RX] Unfreezing tile t=%0t, x=%d, y=%d", $time, my_x_i, my_y_i);
  end
end

// synopsys translate_on

endmodule