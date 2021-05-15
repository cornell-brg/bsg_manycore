bsg_tiles_X = 1
bsg_tiles_Y = 1


all: main.run

# PP: you can override this by appending CGRA_TSET=vvadd_8x8 etc. to
# your make command
CGRA_TEST ?= vvadd_8x8
MAX_CYCLES=50000

RISCV_GCC_EXTRA_OPTS ?= -DTEST_SEL=$(CGRA_TEST)
# VPD_FILENAME ?= $(CGRA_TEST).vpd

CGRA_HEADER_FILES =  cgra_xcel_hb_tapeout.h
CGRA_HEADER_FILES += ./cgra_test_vectors/cgra_test_vectors.dat

OBJECT_FILES=main.o

IGNORE_CADENV=1
WAVE=1

include ../Makefile.include


main.riscv: $(CGRA_HEADER_FILES) $(LINK_SCRIPT) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) $(BSG_MANYCORE_LIB) crt.o
	$(RISCV_LINK) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) -L. "-l:$(BSG_MANYCORE_LIB)" -o $@ $(RISCV_LINK_OPTS)


main.o: Makefile

include ../../mk/Makefile.tail_rules
