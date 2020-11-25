# Single SMU integrated into the manycore

#########################################################
# Network Configutaion
# If not configured, Will use default Values
BSG_MACHINE_GLOBAL_X                  = 16
BSG_MACHINE_GLOBAL_Y                  = 9

#########################################################
#Tile group configuration
# If not configured, Will use default Values
bsg_tiles_org_X = 1
bsg_tiles_org_Y = 1

# If not configured, Will use default Values
	# bsg_tiles_X ?= 2
	# bsg_tiles_Y ?= 2
bsg_tiles_X = 1
bsg_tiles_Y = 1

IGNORE_CADENV = 1
# WAVE = 1
# export VCS=vcs
# export DVE=dve


all: main.run


KERNEL_NAME ?=kernel_smu_xcel

OBJECT_FILES=main.o kernel_smu_xcel.o

include ../../Makefile.include


main.riscv: $(LINK_SCRIPT) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) $(BSG_MANYCORE_LIB) ../../common/crt.o
	$(RISCV_LINK) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) -L. "-l:$(BSG_MANYCORE_LIB)" -o $@ $(RISCV_LINK_OPTS)


main.o: Makefile

include ../../../mk/Makefile.tail_rules
