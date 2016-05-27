`include "bsg_manycore_packet.vh"

module bsg_manycore_proc #(x_cord_width_p   = "inv"
                           , y_cord_width_p = "inv"
                           , data_width_p   = 32
                           , addr_width_p   = 32
                           , packet_width_lp        = `bsg_manycore_packet_width(addr_width_p,data_width_p,x_cord_width_p,y_cord_width_p)
                           , return_packet_width_lp = `bsg_manycore_return_packet_width(x_cord_width_p,y_cord_width_p)
                           , debug_p        = 0
                           , bank_size_p    = "inv" // in words
                           , num_banks_p    = "inv"

                           // this credit counter is more for implementing memory fences
                           // than containing the number of outstanding remote stores

                           //, max_remote_store_credits_p = (1<<13)-1  // 13 bit counter
                           , max_remote_store_credits_p = 200  // 13 bit counter

                           // this is the size of the receive FIFO
                           , proc_fifo_els_p = 4
                           , mem_width_lp    = $clog2(bank_size_p) + $clog2(num_banks_p)
                           , num_nets_lp     = 2
                           )
   (input   clk_i
    , input reset_i

    , input  [num_nets_lp-1:0]                       v_i
    , input  [packet_width_lp-1:0]                data_i
    , input  [return_packet_width_lp-1:0]  return_data_i
    , output [num_nets_lp-1:0]                   ready_o

    , output [num_nets_lp-1:0]                       v_o
    , output [packet_width_lp-1:0]                data_o
    , output [return_packet_width_lp-1:0]  return_data_o
    , input  [num_nets_lp-1:0]                   ready_i

    // tile coordinates
    , input   [x_cord_width_p-1:0]                my_x_i
    , input   [y_cord_width_p-1:0]                my_y_i

    , output logic freeze_o
    );

   `declare_bsg_manycore_packet_s(addr_width_p, data_width_p, x_cord_width_p, y_cord_width_p);

   // input fifo from network

   logic                              cgni_v, cgni_yumi;
   logic [packet_width_lp-1:0] 	      cgni_data;
   wire 			      credit_return_lo;

   bsg_manycore_endpoint #(.x_cord_width_p (x_cord_width_p)
                           ,.y_cord_width_p(y_cord_width_p)
                           ,.fifo_els_p    (proc_fifo_els_p)
                           ,.data_width_p  (data_width_p)
                           ,.addr_width_p  (addr_width_p)
                           ) endp
   (.clk_i
    ,.reset_i
    ,.v_i, .data_i, .return_data_i, .ready_o
    ,.return_v_o(v_o[1]), .return_data_o, .return_ready_i(ready_i[1])

    ,.fifo_data_o(cgni_data)
    ,.fifo_v_o   (cgni_v   )
    ,.fifo_yumi_i(cgni_yumi)

    ,.credit_v_r_o(credit_return_lo)
    );

   // decode incoming packet
   logic                         pkt_freeze, pkt_unfreeze, pkt_remote_store, pkt_unknown;
   logic [data_width_p-1:0]      remote_store_data;
   logic [(data_width_p>>3)-1:0] remote_store_mask;
   logic [addr_width_p-1:0]      remote_store_addr;
   logic                         remote_store_v, remote_store_yumi;


   always_ff @(negedge clk_i)
     assert (~v_i[1] | (return_data_i == {my_y_i, my_x_i}))
       else
	 $error("%m errant credit packet for YX=%d,%d landed at YX=%d,%d",return_data_i[x_cord_width_p+:y_cord_width_p],return_data_i[0+:x_cord_width_p]
		,my_y_i,my_x_i);


   if (debug_p)
   always_ff @(negedge clk_i)
     if (v_o[0])
       $display("%m attempting remote store send of data %x, ready_i = %x",data_o,ready_i[0]);

   if (debug_p)
     always_ff @(negedge clk_i)
       if (cgni_v)
         $display("%m data %x avail on cgni (cgni_yumi=%x,remote_store_v=%x, remote_store_addr=%x, remote_store_data=%x, remote_store_yumi=%x)"
                  ,cgni_data,cgni_yumi,remote_store_v,remote_store_addr, remote_store_data, remote_store_yumi);

   bsg_manycore_pkt_decode #(.x_cord_width_p (x_cord_width_p)
                             ,.y_cord_width_p(y_cord_width_p)
                             ,.data_width_p  (data_width_p )
                             ,.addr_width_p  (addr_width_p )
                             ) pkt_decode
     (.v_i                 (cgni_v)
      ,.data_i             (cgni_data)

      ,.pkt_freeze_o       (pkt_freeze)
      ,.pkt_unfreeze_o     (pkt_unfreeze)
      ,.pkt_unknown_o      (pkt_unknown)

      ,.pkt_remote_store_o (remote_store_v)
      ,.data_o             (remote_store_data)
      ,.addr_o             (remote_store_addr)
      ,.mask_o             (remote_store_mask)
      );

   // deque if we successfully do a remote store, or if it's
   // either kind of packet freeze instruction
   assign cgni_yumi = remote_store_yumi | pkt_freeze | pkt_unfreeze;

   // create freeze gate
   logic                       freeze_r;
   assign freeze_o = freeze_r;

   always_ff @(posedge clk_i)
     if (reset_i)
       freeze_r <= 1'b1;
     else
       if (pkt_freeze | pkt_unfreeze)
         begin
            $display("## freeze_r <= %x",pkt_freeze);
            freeze_r <= pkt_freeze;
         end

   logic [1:0]                         core_mem_v;
   logic [1:0]                         core_mem_w;
   logic [1:0] [addr_width_p-1:0]      core_mem_addr;
   logic [1:0] [data_width_p-1:0]      core_mem_wdata;
   logic [1:0] [(data_width_p>>3)-1:0] core_mem_mask;
   logic [1:0]                         core_mem_yumi;
   logic [1:0]                         core_mem_rv;
   logic [1:0] [data_width_p-1:0]      core_mem_rdata;

   logic core_mem_reserve_1, core_mem_reservation_r;

   logic [addr_width_p-1:0]      core_mem_reserve_addr_r;

   // implement LR (load word reserved)
   always_ff @(posedge clk_i)
     begin
        // if we commit a reserved memory access
        // to the interface, then the reservation takes place
        if (core_mem_v & core_mem_reserve_1 & core_mem_yumi[1])
          begin
             // copy address
             core_mem_reservation_r  <= 1'b1;
             core_mem_reserve_addr_r <= core_mem_addr[1];
             $display("## x,y = %d,%d enabling reservation on %x",my_x_i,my_y_i,core_mem_addr[1]);
          end
        else
          // otherwise, we clear existing reservations if the corresponding
          // address is committed as a remote store
          begin
             if (remote_store_v && (core_mem_reserve_addr_r == remote_store_addr) && remote_store_yumi)
               begin
                  core_mem_reservation_r  <= 1'b0;
                  $display("## x,y = %d,%d clearing reservation on %x",my_x_i,my_y_i,core_mem_reserve_addr_r);
               end
          end
     end

   logic [$clog2(max_remote_store_credits_p+1)-1:0] remote_store_credits;

   wire launching_remote_store = v_o[0] & ready_i[0];

   bsg_vscale_core #(.x_cord_width_p (x_cord_width_p)
                     ,.y_cord_width_p(y_cord_width_p)
                     )
            core
     ( .clk_i   (clk_i)
       ,.reset_i (reset_i)
       ,.freeze_i (freeze_r)

       ,.m_v_o          (core_mem_v)
       ,.m_w_o          (core_mem_w)
       ,.m_addr_o       (core_mem_addr)
       ,.m_data_o       (core_mem_wdata)
       ,.m_reserve_1_o  (core_mem_reserve_1)
       ,.m_reservation_i(core_mem_reservation_r)
       ,.m_mask_o       (core_mem_mask)

       // for data port (1), either the network or the banked memory can
       // deque the item.
       ,.m_yumi_i    ({(launching_remote_store) | core_mem_yumi[1]
                       , core_mem_yumi[0]})
       ,.m_v_i       (core_mem_rv)
       ,.m_data_i    (core_mem_rdata)

       // whether there are any outstanding remote stores
       ,.outstanding_stores_i(remote_store_credits != max_remote_store_credits_p)

       ,.my_x_i (my_x_i)
       ,.my_y_i (my_y_i)
       );


   bsg_counter_up_down #(.max_val_p  (max_remote_store_credits_p)
                         ,.init_val_p(max_remote_store_credits_p)
                         ) remote_store_credit_ctr
     (.clk_i
      ,.reset_i
      ,.down_i   (launching_remote_store)  // launch remote store
      ,.up_i     (credit_return_lo      )  // receive credit back
      ,.count_o(remote_store_credits )
      );

   wire remote_store_request;

   bsg_manycore_pkt_encode #(.x_cord_width_p (x_cord_width_p)
                             ,.y_cord_width_p(y_cord_width_p)
                             ,.data_width_p (data_width_p )
                             ,.addr_width_p (addr_width_p )
                             ) pkt_encode
     (.clk_i(clk_i)

      // the memory request, from the core's data memory port
      ,.v_i    (core_mem_v    [1])
      ,.data_i (core_mem_wdata[1])
      ,.addr_i (core_mem_addr [1])
      ,.we_i   (core_mem_w    [1])
      ,.mask_i (core_mem_mask [1])
      ,.my_x_i (my_x_i)
      ,.my_y_i (my_y_i)
      // directly out to the network!
      ,.v_o    (remote_store_request)
      ,.data_o (data_o)
      );

   // we only request to send a remote store if it would not overflow the remote store credit counter
   assign v_o[0] = remote_store_request & (|remote_store_credits);

   // synopsys translate off

   // this is not an error, but it is extremely surprising
   // and merits investigation

   always @(negedge clk_i)
     assert (remote_store_credits != 0) else
          $error("## %m out of remote store credits %(d)",remote_store_credits);

   bsg_manycore_packet_s data_o_debug;
   assign data_o_debug = data_o;

   if (debug_p)
     always @(negedge clk_i)
       begin
          if (launching_remote_store)
            $display("proc sending packet %x (op=%x, addr=%x, data=%x, y_cord=%x, x_cord=%x), bit_mask=%x, core_mem_wdata=%x, core_mem_addr=%x"
                     , data_o_debug
                     , data_o_debug.op
                     , data_o_debug.addr
                     , data_o_debug.data
                     , data_o_debug.y_cord
                     , data_o_debug.x_cord
                     , core_mem_mask [1]
                     , core_mem_wdata[1]
                     , core_mem_addr [1]
                     );
       end

   // synopsys translate on

   wire [data_width_p-1:0] unused_data;
   wire                    unused_valid;

   // we create dedicated signals for these wires to allow easy access for "bind" statements
   wire [2:0]              xbar_port_v_in = {
                                              // request to write only if we are not sending a remote store packet
                                              // we check the high bit only for performance
                                               core_mem_v[1] & ~core_mem_addr[1][31]
                                              , remote_store_v
                                              , core_mem_v[0]
                                              };

   // proc data port sometimes writes, the network port always writes, proc inst port never writes
   wire [2:0]                    xbar_port_we_in   = { core_mem_w[1], 1'b1, 1'b0};
   wire [2:0]                    xbar_port_yumi_out;
   wire [2:0] [data_width_p-1:0] xbar_port_data_in = { core_mem_wdata [1], remote_store_data, core_mem_wdata[0]};
   wire [2:0] [mem_width_lp-1:0] xbar_port_addr_in = {   core_mem_addr[1]  [2+:mem_width_lp]
                                                       , remote_store_addr [2+:mem_width_lp]
                                                       , core_mem_addr[0]  [2+:mem_width_lp]
                                                       };
   wire [2:0] [(data_width_p>>3)-1:0] xbar_port_mask_in = { core_mem_mask[1], remote_store_mask, core_mem_mask[0] };

   always @(negedge clk_i)
     if (0)
     begin
        if (~freeze_r)
          $display("x=%x y=%x xbar_v_i=%b xbar_w_i=%b xbar_port_yumi_out=%b xbar_addr_i[2,1,0]=%x,%x,%x, xbar_data_i[2,1,0]=%x,%x,%x, xbar_data_o[1,0]=%x,%x"
                   ,my_x_i
                   ,my_y_i
                   ,xbar_port_v_in
                   ,xbar_port_we_in
                   ,xbar_port_yumi_out
                   ,xbar_port_addr_in[2]*4,xbar_port_addr_in[1]*4,xbar_port_addr_in[0]*4
                   ,xbar_port_data_in[2], xbar_port_data_in[1], xbar_port_data_in[0]
                   ,core_mem_rdata[1], core_mem_rdata[0]
                   );
     end

   // the swizzle function changes how addresses are mapped to banks
   wire [2:0] [mem_width_lp-1:0] xbar_port_addr_in_swizzled;
   genvar 			 i;

   for (i = 0; i < 3; i=i+1)
     begin: port
        assign xbar_port_addr_in_swizzled[i] = { xbar_port_addr_in  [i][(mem_width_lp-1)-:1]   // top bit is inst/data
                                                 , xbar_port_addr_in[i][0]                 // and lowest bit determines bank
                                                 , xbar_port_addr_in[i][1]                 // and lowest bit determines bank
                                                 , xbar_port_addr_in[i][2+:(mem_width_lp-2)]
                                                 };

     end

   assign { core_mem_yumi[1], remote_store_yumi, core_mem_yumi[0] } = xbar_port_yumi_out;

   // potentially, we could get better bandwidth if we demultiplexed the remote store input port
   // into four two-element fifos, one per bank. then, the arb could arbitrate for
   // each bank using those fifos. this allows for reordering of remote_stores across
   // banks, eliminating head-of-line blocking on a bank conflict. however, this would eliminate our
   // guaranteed in-order delivery and violate sequential consistency; so it would require some
   // extra hw to enforce that; and tagging of memory fences inside packets.
   // we could most likely get rid of the cgni input fifo in this case.

  bsg_mem_banked_crossbar #
    (.num_ports_p  (3)
     ,.num_banks_p  (num_banks_p)
     ,.bank_size_p  (bank_size_p)
     ,.data_width_p (data_width_p)
//     ,.rr_lo_hi_p   (2'b10) // round robin
//     ,.rr_lo_hi_p   (2'b01) // deadlock
     ,.rr_lo_hi_p(0)          // local dmem has priority
     ,.debug_p(debug_p*4)  // mbt: debug, multiply addresses by 4.
//      ,.debug_p(4)
//     ,.debug_reads_p(0)
    ) banked_crossbar
    ( .clk_i    (clk_i)
     ,.reset_i  (reset_i)
      ,.v_i     (xbar_port_v_in)

      ,.w_i     (xbar_port_we_in)
      ,.addr_i  (xbar_port_addr_in_swizzled)
      ,.data_i  (xbar_port_data_in)
      ,.mask_i  (xbar_port_mask_in)

      // whether the crossbar accepts the input
     ,.yumi_o  ( xbar_port_yumi_out                                     )
     ,.v_o     ({ core_mem_rv    [1], unused_valid, core_mem_rv    [0] })
     ,.data_o  ({ core_mem_rdata [1], unused_data,  core_mem_rdata [0] })
    );




endmodule
