# This file contains a list of the *core* manycore architecture files that are
# used in simulation. This has NOT been used in tapeout or any tapeout related
# activities beyond simulation

VINCLUDES += $(BASEJUMP_STL_DIR)/bsg_misc
VINCLUDES += $(BASEJUMP_STL_DIR)/bsg_cache
VINCLUDES += $(BASEJUMP_STL_DIR)/bsg_noc
VINCLUDES += $(BASEJUMP_STL_DIR)/bsg_tag
# PP: SDR
VINCLUDES += $(BASEJUMP_STL_DIR)/bsg_link
VINCLUDES += $(BIGBLADE_DIR)/common/sdr/sdr_horizontal/v
VINCLUDES += $(BSG_MANYCORE_DIR)/v
VINCLUDES += $(BSG_MANYCORE_DIR)/v/vanilla_bean
VINCLUDES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source
VINCLUDES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/RISCV

VHEADERS += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_defines.v
VHEADERS += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_noc_pkg.v
VHEADERS += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_mesh_router_pkg.v
VHEADERS += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_wormhole_router_pkg.v
VHEADERS += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_noc_links.vh
VHEADERS += $(BASEJUMP_STL_DIR)/bsg_tag/bsg_tag_pkg.v
VHEADERS += $(BASEJUMP_STL_DIR)/bsg_cache/bsg_cache_pkg.v
VHEADERS += $(BASEJUMP_STL_DIR)/bsg_cache/bsg_cache_non_blocking_pkg.v
VHEADERS += $(BASEJUMP_STL_DIR)/bsg_fpu/bsg_fpu_defines.vh
VHEADERS += $(BSG_MANYCORE_DIR)/v/bsg_manycore_pkg.v
VHEADERS += $(BSG_MANYCORE_DIR)/v/vanilla_bean/bsg_vanilla_pkg.v
VHEADERS += $(BSG_MANYCORE_DIR)/v/bsg_manycore_addr_pkg.v
VHEADERS += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/bsg_hardfloat_pkg.v

VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_less_than.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_reduce.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_abs.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_mul_synth.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_priority_encode.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_priority_encode_one_hot_out.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_encode_one_hot.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_scan.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_mux.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_mux_one_hot.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_mux_segmented.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_mux_bitwise.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_dff.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_dff_chain.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_dff_en_bypass.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_dff_reset_en_bypass.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_dff_en.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_dff_reset.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_dff_reset_en.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_dff_reset_set_clear.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_transpose.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_crossbar_o_by_i.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_decode_with_v.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_decode.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_counter_clear_up.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_counter_up_down.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_counter_set_down.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_round_robin_arb.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_arb_round_robin.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_circular_ptr.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_imul_iterative.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_idiv_iterative.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_idiv_iterative_controller.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_inv.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_buf.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_buf_ctrl.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_xnor.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_nor2.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_adder_cin.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_expand_bitmask.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_lru_pseudo_tree_decode.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_lru_pseudo_tree_encode.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_lru_pseudo_tree_backup.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_thermometer_count.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_id_pool.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_concentrate_static.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_array_concentrate_static.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_unconcentrate_static.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_mux2_gatestack.v


VHEADERS += $(BASEJUMP_STL_DIR)/bsg_tag/bsg_tag_client.v

VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_fifo_1r1w_large.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_fifo_1rw_large.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_fifo_1r1w_small.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_fifo_1r1w_small_unhardened.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_two_fifo.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_round_robin_n_to_1.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_round_robin_2_to_2.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_parallel_in_serial_out.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_serial_in_parallel_out.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_fifo_tracker.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_make_2D_array.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_flatten_2D_array.v

VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1r1w.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1r1w_synth.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1r1w_sync.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1r1w_sync_synth.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1rw_sync.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1rw_sync_synth.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_2r1w_sync.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_2r1w_sync_synth.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_3r1w_sync.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_3r1w_sync_synth.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1rw_sync_mask_write_byte.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1rw_sync_mask_write_byte_synth.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1rw_sync_mask_write_bit.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_mem/bsg_mem_1rw_sync_mask_write_bit_synth.v

VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_mesh_stitch.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_mesh_router.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_mesh_router_decoder_dor.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_mesh_router_buffered.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_wormhole_router.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_wormhole_router_decoder_dor.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_wormhole_router_input_control.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_wormhole_router_output_control.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_wormhole_concentrator.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_wormhole_concentrator_in.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_noc/bsg_wormhole_concentrator_out.v

VSOURCES += $(BASEJUMP_STL_DIR)/bsg_async/bsg_launch_sync_sync.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_async/bsg_sync_sync.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_async/bsg_async_fifo.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_async/bsg_async_ptr_gray.v

VSOURCES += $(BASEJUMP_STL_DIR)/bsg_cache/bsg_cache.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_cache/bsg_cache_decode.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_cache/bsg_cache_dma.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_cache/bsg_cache_miss.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_cache/bsg_cache_sbuf.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_cache/bsg_cache_sbuf_queue.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_link_to_cache.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_vcache_blocking.v


VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/fNToRecFN.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/compareRecFN.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/divSqrtRecFN_small.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/iNToRecFN.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/mulAddRecFN.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/recFNToFN.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/HardFloat_rawFN.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/HardFloat_primitives.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/isSigNaNRecFN.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/recFNToIN.v
VSOURCES += $(BSG_MANYCORE_DIR)/imports/HardFloat/source/RISCV/HardFloat_specialize.v


VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/bsg_manycore_proc_vanilla.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/network_rx.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/network_tx.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/vanilla_core.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/alu.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/cl_decode.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fpu_float.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fpu_float_fma.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fpu_float_fma_round.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fpu_float_aux.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fpu_int.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fpu_int_fclass.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fcsr.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/mcsr.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fpu_fdiv_fsqrt.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/fpu_fmin_fmax.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/icache.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/idiv.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/load_packer.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/lsu.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/regfile.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/regfile_synth.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/regfile_hard.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/vanilla_bean/scoreboard.v

VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_pod_ruche_array.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_pod_ruche.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_pod_ruche_row.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_tile_compute_array_ruche.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_tile_compute_ruche.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_tile_vcache_array.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_tile_vcache.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_cache_dma_to_wormhole.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_hetero_socket.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_mesh_node.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_endpoint.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_endpoint_standard.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_lock_ctrl.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_reg_id_decode.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_1hold.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_eva_to_npa.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_dram_hash_function.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_link_sif_tieoff.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_ruche_x_link_sif_tieoff.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_ruche_buffer.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_ruche_anti_buffer.v

# PP: SDR dependencies -- from sdr_horizontal/tcl/filelist.tcl
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_xor.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_async/bsg_async_credit_counter.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_array_reverse.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_gray_to_binary.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_binary_plus_one_to_gray.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_wait_cycles.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_misc/bsg_counter_up_down_variable.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_channel_tunnel.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_channel_tunnel_in.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_channel_tunnel_out.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_1_to_n_tagged_fifo.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_1_to_n_tagged.v
# VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_round_robin_1_to_n.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_fifo_1r1w_pseudo_large.v
# VSOURCES += $(BASEJUMP_STL_DIR)/bsg_dataflow/bsg_serial_in_parallel_out_full.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_fsb/bsg_fsb_pkg.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_link/bsg_link_sdr.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_link/bsg_link_sdr_downstream.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_link/bsg_link_sdr_upstream.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_link/bsg_link_isdr_phy.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_link/bsg_link_osdr_phy.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_link/bsg_link_osdr_phy_phase_align.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_link/bsg_link_source_sync_downstream.v
VSOURCES += $(BASEJUMP_STL_DIR)/bsg_link/bsg_link_source_sync_upstream_sync.v
VSOURCES += $(BIGBLADE_DIR)/common/sdr/sdr_horizontal/v/bsg_manycore_link_ruche_to_sdr_east.v

# PP: hor io routers
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_hor_io_router.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/bsg_manycore_hor_io_router_column.v

# PP: CGRAXcel sources
VSOURCES += $(BSG_MANYCORE_DIR)/v/brg_cgra_xcel/HBEndpointCGRAXcel_8x8Array_4x4KBSpads__pickled.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/brg_cgra_xcel/brg_8x8_cgra_xcel.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/brg_cgra_xcel/brg_cgra_pod_sync.v
VSOURCES += $(BSG_MANYCORE_DIR)/v/brg_cgra_xcel/brg_cgra_pod.v

# PP: Include SRAM verilog model for RTL hard simulation
# Update: we don't need this if we only want to perform RTL-soft sims
# VSOURCES += /work/global/secure/en-ec-brg-vip-gf-14nm-14lppxl-nda/hb-chip/sram/*.v
