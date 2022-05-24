//========================================================================
// stats.h
//========================================================================

#ifndef APPLRTS_STATS_H
#define APPLRTS_STATS_H

// stats enable
#define APPL_STATS

#ifdef APPL_STATS

#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "appl-hw-barrier.hpp"

namespace applrts {
namespace stats {
extern uint32_t s_task_enqueue;
extern uint32_t s_task_dequeue;
extern uint32_t s_task_stolen;
extern uint32_t s_stealing_attempt;
}} // applrts::stats

#endif // APPL_STATS

namespace applrts {
namespace stats {

inline void log_task_enqueue() {
#ifdef APPL_STATS
  s_task_enqueue++;
#endif
}

inline void log_task_dequeue() {
#ifdef APPL_STATS
  s_task_dequeue++;
#endif
}

inline void log_task_stolen() {
#ifdef APPL_STATS
  s_task_stolen++;
#endif
}

inline void log_stealing_attempt() {
#ifdef APPL_STATS
  s_stealing_attempt++;
#endif
}

__attribute__ ((noinline)) void dump_stats();

}} // applrts::stats

#endif
