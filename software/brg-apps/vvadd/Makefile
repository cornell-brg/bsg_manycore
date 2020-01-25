#########################################################
# Network Configutaion
# If not configured, Will use default Values
		bsg_global_X ?= $(bsg_tiles_X)
		bsg_global_Y ?= $(bsg_tiles_Y)+1

#########################################################
#Tile group configuration
# If not configured, Will use default Values
		bsg_tiles_org_X ?= 0
		bsg_tiles_org_Y ?= 1

# If not configured, Will use default Values
		bsg_tiles_X ?= 2
		bsg_tiles_Y ?= 2


all: main.run


KERNEL_NAME ?=kernel_vvadd

OBJECT_FILES=main.o bsg_set_tile_x_y.o bsg_printf.o kernel_vvadd.o

main.riscv: $(OBJECT_FILES) crt.o
		$(RISCV_LINK) $(OBJECT_FILES) -o $@ $(RISCV_LINK_OPTS)


main.o: Makefile

include ../Makefile.include

include $(BSG_MANYCORE_DIR)/software/mk/Makefile.tail_rules
