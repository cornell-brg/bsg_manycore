#include "applrts-stats.hpp"

namespace applrts {
namespace stats {

uint32_t s_task_enqueue;
uint32_t s_task_dequeue;
uint32_t s_task_stolen;
uint32_t s_stealing_attempt;

__attribute__ ((noinline)) void dump_stats() {
#ifdef APPL_STATS
  for (uint32_t cid = 0; cid < bsg_tiles_X * bsg_tiles_Y; cid++) {
    if (cid == __bsg_id) {
      bsg_print_int(1200);
      bsg_print_int(__bsg_id);
      bsg_print_int(s_task_enqueue);
      bsg_print_int(s_task_dequeue);
      bsg_print_int(s_task_stolen);
      bsg_print_int(s_stealing_attempt);
    }
    // use a sync to serial prints
    // make sure all tiles call this dump
    appl::sync();
  }
#endif
}

}} // applrts::stats

