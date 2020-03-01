#########################################################
# Network Configutaion
# If not configured, Will use default Values
bsg_global_X = 4
bsg_global_Y = 4

#########################################################
#Tile group configuration
# If not configured, Will use default Values
bsg_tiles_org_X = 1
bsg_tiles_org_Y = 1

# If not configured, Will use default Values
	# bsg_tiles_X ?= 2
	# bsg_tiles_Y ?= 2
bsg_tiles_X = 2
bsg_tiles_Y = 2

IGNORE_CADENV = 1
# WAVE = 1
# export VCS=vcs
# export DVE=dve


all: main.run


KERNEL_NAME ?=kernel_vec_add_xcel

OBJECT_FILES=main.o bsg_set_tile_x_y.o bsg_printf.o kernel_vec_add_xcel.o

include ../../Makefile.include


main.riscv: $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) ../../common/crt.o
	$(RISCV_LINK) $(OBJECT_FILES) $(SPMD_COMMON_OBJECTS) -o $@ $(RISCV_LINK_OPTS)


main.o: Makefile

include ../../../mk/Makefile.tail_rules
