#include <map>
#include <cstdint>
#include "vanilla_core_dmem_tracker.h"

struct vanilla_core_dmem_tracker {
public:
    // constructors
    vanilla_core_dmem_tracker(){}
    virtual ~vanilla_core_dmem_tracker(){}
    
public:
    // members    
    std::map<pc_t,addr_t> pc2addr;
};

void *vanilla_core_dmem_tracker_new() {
    return new vanilla_core_dmem_tracker;
}

void  vanilla_core_dmem_tracker_delete(void *ptr) {
    vanilla_core_dmem_tracker* dmem_trckr = reinterpret_cast<vanilla_core_dmem_tracker*>(ptr);
    delete dmem_trckr;
}

void vanilla_core_dmem_tracker_post_write(void *ptr, pc_t pc, addr_t addr) {
    vanilla_core_dmem_tracker* dmem_trckr = reinterpret_cast<vanilla_core_dmem_tracker*>(ptr);
    // record if first write
    if (dmem_trckr->pc2addr.find(addr) == dmem_trckr->pc2addr.end()) {
        dmem_trckr->pc2addr[addr] = pc;
    }
}
