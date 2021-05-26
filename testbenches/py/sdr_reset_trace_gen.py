from bsg_tag_trace_gen import *
import sys

if __name__ == "__main__":

  # instantiate tg
  num_masters_p = 2
  num_clients_p = 1024
  max_payload_width_p = 9
  tg = TagTraceGen(num_masters_p, num_clients_p, max_payload_width_p)

  # parameters
  clk_num_clients_p  = 5
  sdr_num_clients_p  = 4
  link_num_clients_p = 2*clk_num_clients_p+2
  noc_num_clients_p  = 2*link_num_clients_p+sdr_num_clients_p+1

  link_offset    = 0
  row_sdr_offset = link_offset+noc_num_clients_p*9
  row_pod_offset = row_sdr_offset+4*sdr_num_clients_p*4
  clk_gen_offset = row_pod_offset+4*1*4


  # reset all bsg_tag_master
  tg.send(masters=0b11, client_id=0, data_not_reset=0, length=0, data=0)

  # wait 64 cycles
  tg.wait(64)



  # reset clk_gen
  # for noc in range(9):
  #   for link in range(2):
  #     for clk in range(2):
  #       offset = link_offset+(noc*noc_num_clients_p)+(link*link_num_clients_p)+(clk*clk_num_clients_p)
  #       tg.send(masters=0b11, client_id=2+offset, data_not_reset=0, length=1, data=0b1)
  #       tg.send(masters=0b11, client_id=3+offset, data_not_reset=0, length=5, data=0b11111)
  #       tg.send(masters=0b11, client_id=4+offset, data_not_reset=0, length=1, data=0b1)
  #       tg.send(masters=0b11, client_id=5+offset, data_not_reset=0, length=7, data=0b1111111)
  #       tg.send(masters=0b11, client_id=6+offset, data_not_reset=0, length=2, data=0b11)



  # config clk_gen output
  # for noc in range(9):
  #   for link in range(2):
  #     for clk in range(2):
  #       offset = link_offset+(noc*noc_num_clients_p)+(link*link_num_clients_p)+(clk*clk_num_clients_p)
  #       if len(sys.argv) == 2 and (sys.argv[1] == 'use_clk_gen'):

  #         # select zero output clk
  #         tg.send(masters=0b11, client_id=6+offset, data_not_reset=1, length=2, data=0b11)
          
  #         # reset oscillator and trigger flops
  #         tg.send(masters=0b11, client_id=2+offset, data_not_reset=1, length=1, data=0b1)
          
  #         # init trigger to low, init oscillator to zero
  #         # OSC INIT VALUE MUST BE ZERO TO AVOID X IN SIMULATION
  #         tg.send(masters=0b11, client_id=4+offset, data_not_reset=1, length=1, data=0b0)
  #         tg.send(masters=0b11, client_id=3+offset, data_not_reset=1, length=5, data=0b00000)
          
  #         # take oscillator and trigger flops out of reset
  #         tg.send(masters=0b11, client_id=2+offset, data_not_reset=1, length=1, data=0b0)
          
  #         # trigger oscillator value
  #         tg.send(masters=0b11, client_id=4+offset, data_not_reset=1, length=1, data=0b1)
  #         tg.send(masters=0b11, client_id=4+offset, data_not_reset=1, length=1, data=0b0)
          
  #         # reset ds, then set ds value
  #         tg.send(masters=0b11, client_id=5+offset, data_not_reset=1, length=7, data=0b0000001)
  #         tg.send(masters=0b11, client_id=5+offset, data_not_reset=1, length=7, data=0b0000000)
          
  #         # select ds output clk
  #         tg.send(masters=0b11, client_id=6+offset, data_not_reset=1, length=2, data=0b01)

  #         # set ds value
  #         tg.send(masters=0b11, client_id=5+offset, data_not_reset=1, length=7, data=0b0000010)
          
  #         # set oscillator value, then trigger
  #         tg.send(masters=0b11, client_id=3+offset, data_not_reset=1, length=5, data=0b11000)
  #         tg.send(masters=0b11, client_id=4+offset, data_not_reset=1, length=1, data=0b1)
  #         tg.send(masters=0b11, client_id=4+offset, data_not_reset=1, length=1, data=0b0)

  #       else:

  #         # select ext output clk
  #         tg.send(masters=0b11, client_id=6+offset, data_not_reset=1, length=2, data=0b10)


  # reset noc blocks
  # for noc in range(9):
    # offset = link_offset+(noc*noc_num_clients_p)
  offset = 0
    # reset noc clients
    # tg.send(masters=0b11, client_id=24+offset, data_not_reset=0, length=1, data=0b1)
    # reset sdr clients
  tg.send(masters=0b11, client_id=25+offset, data_not_reset=0, length=1, data=0b1)
  tg.send(masters=0b11, client_id=26+offset, data_not_reset=0, length=1, data=0b1)
  tg.send(masters=0b11, client_id=27+offset, data_not_reset=0, length=1, data=0b1)
  tg.send(masters=0b11, client_id=28+offset, data_not_reset=0, length=1, data=0b1)
    # for link in range(2):
    #   offset = link_offset+(noc*noc_num_clients_p)+(link*link_num_clients_p)
    #   # reset link clients
    #   tg.send(masters=0b11, client_id=0+offset, data_not_reset=0, length=3, data=0b111)
    #   tg.send(masters=0b11, client_id=1+offset, data_not_reset=0, length=2, data=0b11)
      
  # reset pod rows
  # for row in range(4):
  #   for corner in range(4):
  #     offset = row_sdr_offset+(row*4+corner)*sdr_num_clients_p
  #     # reset sdr clients
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=0, length=1, data=0b1)
  #     tg.send(masters=0b11, client_id=1+offset, data_not_reset=0, length=1, data=0b1)
  #     tg.send(masters=0b11, client_id=2+offset, data_not_reset=0, length=1, data=0b1)
  #     tg.send(masters=0b11, client_id=3+offset, data_not_reset=0, length=1, data=0b1)
  #   for pod in range(4):
  #     offset = row_pod_offset+(row*4+pod)
  #     # reset pod clients
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=0, length=1, data=0b1)


  # STEP 1: initialize everything
  # for noc in range(9):
    # offset = link_offset+(noc*noc_num_clients_p)
  offset = 0
    # reset noc clients
    # tg.send(masters=0b11, client_id=24+offset, data_not_reset=1, length=1, data=0b1)
    # reset sdr clients
  tg.send(masters=0b11, client_id=25+offset, data_not_reset=1, length=1, data=0b0)
  tg.send(masters=0b11, client_id=26+offset, data_not_reset=1, length=1, data=0b1)
  tg.send(masters=0b11, client_id=27+offset, data_not_reset=1, length=1, data=0b1)
  tg.send(masters=0b11, client_id=28+offset, data_not_reset=1, length=1, data=0b1)
    # for link in range(2):
    #   offset = link_offset+(noc*noc_num_clients_p)+(link*link_num_clients_p)
    #   # reset link clients
    #   tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=3, data=0b110)
    #   tg.send(masters=0b11, client_id=1+offset, data_not_reset=1, length=2, data=0b11)

  # for row in range(4):
  #   for corner in range(4):
  #     offset = row_sdr_offset+(row*4+corner)*sdr_num_clients_p
  #     # reset sdr clients
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=1, data=0b0)
  #     tg.send(masters=0b11, client_id=1+offset, data_not_reset=1, length=1, data=0b1)
  #     tg.send(masters=0b11, client_id=2+offset, data_not_reset=1, length=1, data=0b1)
  #     tg.send(masters=0b11, client_id=3+offset, data_not_reset=1, length=1, data=0b1)
  #   for pod in range(4):
  #     offset = row_pod_offset+(row*4+pod)
  #     # reset pod clients
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=1, data=0b1)


  # STEP 2: perform async token reset
  # async token reset for io_link
  # for noc in range(9):
  #   for link in range(2):
  #     offset = link_offset+(noc*noc_num_clients_p)+(link*link_num_clients_p)
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=3, data=0b111)
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=3, data=0b110)



  # STEP 3: de-assert upstream io reset
  # de-assert upstream reset for io_link
  # for noc in range(9):
  #   for link in range(2):
  #     offset = link_offset+(noc*noc_num_clients_p)+(link*link_num_clients_p)
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=3, data=0b010)



  # STEP 4: de-assert downstream io reset
  # de-assert downstream reset for io_link
  # for noc in range(9):
  #   for link in range(2):
  #     offset = link_offset+(noc*noc_num_clients_p)+(link*link_num_clients_p)
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=3, data=0b000)



  # STEP 5/6: de-assert upstream/downstream core reset
  # de-assert core reset for io_link
  # for noc in range(9):
  #   for link in range(2):
  #     offset = link_offset+(noc*noc_num_clients_p)+(link*link_num_clients_p)
  #     tg.send(masters=0b11, client_id=1+offset, data_not_reset=1, length=2, data=0b00)



  # STEP 7: SDR perform async token reset
  # for noc in range(9):
    # offset = link_offset+(noc*noc_num_clients_p)
  offset = 0
  tg.send(masters=0b11, client_id=25+offset, data_not_reset=1, length=1, data=0b1)
  tg.send(masters=0b11, client_id=25+offset, data_not_reset=1, length=1, data=0b0)
  # for row in range(4):
  #   for corner in range(4):
  #     offset = row_sdr_offset+(row*4+corner)*sdr_num_clients_p
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=1, data=0b1)
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=1, data=0b0)



  # STEP 8: SDR de-assert uplink reset
  # for noc in range(9):
    # offset = link_offset+(noc*noc_num_clients_p)
  offset = 0
  tg.send(masters=0b11, client_id=28+offset, data_not_reset=1, length=1, data=0b0)
  # for row in range(4):
  #   for corner in range(4):
  #     offset = row_sdr_offset+(row*4+corner)*sdr_num_clients_p
  #     tg.send(masters=0b11, client_id=3+offset, data_not_reset=1, length=1, data=0b0)



  # STEP 9: SDR de-assert downlink reset
  # for noc in range(9):
    # offset = link_offset+(noc*noc_num_clients_p)
  offset = 0
  tg.send(masters=0b11, client_id=27+offset, data_not_reset=1, length=1, data=0b0)
  # for row in range(4):
  #   for corner in range(4):
  #     offset = row_sdr_offset+(row*4+corner)*sdr_num_clients_p
  #     tg.send(masters=0b11, client_id=2+offset, data_not_reset=1, length=1, data=0b0)



  # STEP 10: SDR de-assert downstream reset
  # for noc in range(9):
    # offset = link_offset+(noc*noc_num_clients_p)
  offset = 0
  tg.send(masters=0b11, client_id=26+offset, data_not_reset=1, length=1, data=0b0)
  # for row in range(4):
  #   for corner in range(4):
  #     offset = row_sdr_offset+(row*4+corner)*sdr_num_clients_p
  #     tg.send(masters=0b11, client_id=1+offset, data_not_reset=1, length=1, data=0b0)



  # STEP 11: de-assert noc reset
  # for noc in range(9):
  #   offset = link_offset+(noc*noc_num_clients_p)
  #   tg.send(masters=0b11, client_id=24+offset, data_not_reset=1, length=1, data=0b0)



  # STEP 12: de-assert pod reset
  # de-assert reset for core
  # for row in range(4):
  #   for pod in range(4):
  #     offset = row_pod_offset+(row*4+pod)
  #     tg.send(masters=0b11, client_id=0+offset, data_not_reset=1, length=1, data=0b0)



  tg.wait(64)
  tg.done()
