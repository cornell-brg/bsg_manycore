bsg_tiles_X = 1
bsg_tiles_Y = 1
BSG_MACHINE_PATH = $(BSG_MANYCORE_DIR)/machines/4x4_brg_hcc
LINK_SCRIPT = $(BSG_MANYCORE_DIR)/software/spmd/common/brg_hcc.ld
NBF_PY = $(BSG_MANYCORE_DIR)/software/py/nbf_brg_hcc.py

all: main.run

OBJECT_FILES=main.o

include ../Makefile.include

main.riscv: $(LINK_SCRIPT) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) $(BSG_MANYCORE_LIB) crt.o
	$(RISCV_LINK) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) -L. "-l:$(BSG_MANYCORE_LIB)" -o $@ $(RISCV_LINK_OPTS)

main.o: Makefile

include ../../mk/Makefile.tail_rules
