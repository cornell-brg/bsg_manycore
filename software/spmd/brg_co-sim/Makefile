
CXX = g++
VCS = vcs -sverilog
CPPFLAGS = -m64 -I src/common -I.
LDFLAGS  = -lpthread -ldl -lrt
# VCS_HOME = /opt/synopsys/packages/vcs/N-2017.12-1

bsg_tiles_X = 1
bsg_tiles_Y = 1

# For host - device co-som
DEVICE = 0

all: main.run 

host_lib.o : src/host/host_lib.cc src/host/host_lib.h
	$(CXX) $(CPPFLAGS) -c $< -o $@

device_ipc.so : src/device/device_ipc.cc
	$(CXX) $(CPPFLAGS) -shared -fPIC -I$(VCS_HOME)/include $< -o $@ $(LDFLAGS)

test_host : src/host/test_host.cc host_lib.o
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

test_host_orig : src/host/test_host_orig.cc host_lib.o
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

# test_device : src/device/brg_co_sim.v device_ipc.so
# 	$(VCS) $^ -o $@

# Rule to write processor execution logs. To be used after the
# verilog simulation.
#
# Redirects verilog standard output starting with "X<x_cord>_Y<y_cord>.pelog" 
# to a unique log file for each coordinate in the manycore. This can be useful 
# for a quick debug of processor or program running on it.
proc_exe_logs: X0_Y0.pelog X1_Y0.pelog

OBJECT_FILES=main.o bsg_set_tile_x_y.o bsg_printf.o

include ../Makefile.include

#########################################################
#            FPU OPTION
#     The number of horizon node must be two and must 
#     be vanilla core 
BSG_FPU_OP=0

main.riscv: $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) ../common/crt.o
	$(RISCV_LINK) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) -o $@ $(RISCV_LINK_OPTS)
	


main.o: Makefile device_ipc.so test_host 

include ../../mk/Makefile.tail_rules
