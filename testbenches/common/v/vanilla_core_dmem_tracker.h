#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned addr_t;
typedef unsigned pc_t;

void *vanilla_core_dmem_tracker_new();
void  vanilla_core_dmem_tracker_delete(void *ptr);
void  vanilla_core_dmem_tracker_post_write(void *ptr, pc_t pc, addr_t addr);

#ifdef __cplusplus
}
#endif
