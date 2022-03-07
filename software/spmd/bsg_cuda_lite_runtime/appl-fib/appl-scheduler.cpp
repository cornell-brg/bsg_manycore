//========================================================================
// Scheduler.cpp
//========================================================================
// A standard work-stealing scheduler.

#include "appl-scheduler.hpp"

namespace appl {
namespace local {

SimpleDeque<Task*>* g_taskq_p;

} // namespace local
} // namespace appl

