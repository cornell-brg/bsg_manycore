module brg_hcc_tile
    import bsg_manycore_pkg::*;
#(
   x_cord_width_p                 = "inv"
  ,y_cord_width_p                 = "inv"
  ,dmem_size_p                    = "inv"
  ,data_width_p                   = 32
  ,addr_width_p                   = 32
  ,max_out_credits_p              = 200
  ,packet_width_lp                = `bsg_manycore_packet_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
  ,return_packet_width_lp         = `bsg_manycore_return_packet_width(x_cord_width_p,y_cord_width_p,data_width_p)
  ,bsg_manycore_link_sif_width_lp = `bsg_manycore_link_sif_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
  ,debug_p                        = 1
  ,hetero_type_p                  = 1
  ,epa_byte_addr_width_p          = "inv"
)
(
   input clk_i
  ,input reset_i

  // mesh network
  ,input  [bsg_manycore_link_sif_width_lp-1:0] link_sif_i
  ,output [bsg_manycore_link_sif_width_lp-1:0] link_sif_o
  ,input  [x_cord_width_p-1:0] my_x_i
  ,input  [y_cord_width_p-1:0] my_y_i

  // Dummy outputs to be compatilbe with the socket
  ,output freeze_o
);

logic [31:0] flat_id;

logic                                   out_v;
logic [packet_width_lp-1:0]             out_packet;
logic                                   out_ready;
logic [$clog2(max_out_credits_p+1)-1:0] out_credits;

logic [data_width_p-1:0] returned_data_r;
logic                    returned_v_r;
logic [4:0]              returned_reg_id_r;
logic [1:0]              returned_pkt_type_r;
logic                    returned_fifo_full;
logic                    returned_yumi;

assign flat_id = my_x_i * my_y_i;

//------------------------------------------------------------------------
// BrgHBTile
//------------------------------------------------------------------------

BrgHBTile tile (
   .clk    (clk_i)
  ,.reset  (reset_i)
  ,.tileid (flat_id)
  // not used
  ,.go_ifc__en     (1'd0)
  ,.go_ifc__msg    (1'd0)
  ,.test_sink__rdy (1'd0)
  ,.test_src__en   (1'd0)
  ,.test_src__msg  ('d0)
  // out_request
  ,.out_ready  (out_ready)
  ,.out_v      (out_v)
  ,.out_packet (out_packet)
  // in_response
  ,.returned_data_r     (returned_data_r)
  ,.returned_fifo_full  (returned_fifo_full)
  ,.returned_pkt_type_r (returned_pkt_type_r)
  ,.returned_reg_id_r   (returned_reg_id_r)
  ,.returned_v_r        (returned_v_r)
  ,.returned_yumi       (returned_yumi)
);

bsg_manycore_endpoint_standard
#(
   .x_cord_width_p    (x_cord_width_p)
  ,.y_cord_width_p    (y_cord_width_p)
  ,.fifo_els_p        (4)
  ,.data_width_p      (data_width_p)
  ,.addr_width_p      (addr_width_p)
  ,.max_out_credits_p (max_out_credits_p)
) endpoint_gs
(  .clk_i   (clk_i)
  ,.reset_i (reset_i)

  ,.my_x_i (my_x_i)
  ,.my_y_i (my_y_i)

  // mesh network
  ,.link_sif_i (link_sif_i)
  ,.link_sif_o (link_sif_o)
  ,.my_x_i     (my_x_i)
  ,.my_y_i     (my_y_i)

  // master request (out_request)
  ,.out_v_i               (out_v)
  ,.out_packet_i          (out_packet)
  ,.out_credit_or_ready_o (out_ready)
  ,.out_credits_o         (out_credits)

  // master reponse (in_response)
  ,.returned_data_r_o     (returned_data_r)
  ,.returned_reg_id_r_o   (returned_reg_id_r)
  ,.returned_v_r_o        (returned_v_r)
  ,.returned_pkt_type_r_o (returned_pkt_type)
  ,.returned_yumi_i       (returned_yumi)
  ,.returned_fifo_full_o  (returned_fifo_full)

  // slave request (in_request), tied-down
  ,.in_yumi_i (1'd0)

  // slave response (out_response), tied-down
  ,.returning_data_i ('d0)
  ,.returning_v_i    (1'd0)
);

endmodule