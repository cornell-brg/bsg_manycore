#=======================================================================
# cgra_hpod_trace_gen.py
#=======================================================================
# Generate half pod trace file.

from bsg_tag_trace_gen import *
import sys
import math

if __name__ == "__main__":
  num_pods_x = int(sys.argv[1])
  num_pods_y = int(sys.argv[2])

  # each hpod has 4 clients
  num_clients = num_pods_y*4
  payload_width = 7 # y_cord_width_p
  lg_payload_width = int(math.ceil(math.log(payload_width+1,2)))
  max_payload_width = (1<<lg_payload_width)-1
  tg = TagTraceGen(1, num_clients, max_payload_width)

  # reset all bsg_tag_master
  tg.send(masters=0b1,client_id=0,data_not_reset=0,length=0,data=0)
  tg.wait(32)
  
  # client indexing [num_pods_y-1:0][3:0]
  # reset all clients
  for i in range(num_clients):
    tg.send(masters=0b1, client_id=i, data_not_reset=0, length=max_payload_width, data=(2**max_payload_width)-1)
    
  # Feed in the correct Y coordinates
  for i in range(num_pods_y):
    for j in range(4):
      y_cord = ((i*2+1) << 3) + j
      tg.send(masters=0b1, client_id=i*4+j, data_not_reset=1, length=payload_width, data=y_cord)

  # Done
  tg.wait(16)
  tg.done()
