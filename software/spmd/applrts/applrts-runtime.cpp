//========================================================================
// runtime.inl
//========================================================================

#include "applrts-runtime.hpp"

namespace applrts {

namespace local {

SimpleDeque<Task*> g_taskq = SimpleDeque<Task*>();
bool seed_enable;
Task* seed_task;
Task** seed_target;

} // namespace local

} // namespace applrts
