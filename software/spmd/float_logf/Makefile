bsg_tiles_X = 1
bsg_tiles_Y = 1


all: main.run



OBJECT_FILES=main.o

include ../Makefile.include

# -fno-finite-math-only is required when using -ffast-math (see
# -Makefile.builddefs) and using isnan/isfinite
RISCV_GCC_OPTS += -fno-threadsafe-statics -ffp-contract=off -lm -g -fno-finite-math-only
RISCV_LINK_OPTS += -lc -lm

main.riscv: $(LINK_SCRIPT) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) $(BSG_MANYCORE_LIB) crt.o
	$(RISCV_LINK) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) -L. "-l:$(BSG_MANYCORE_LIB)" -o $@ $(RISCV_LINK_OPTS)


main.o: Makefile

include ../../mk/Makefile.tail_rules
