//========================================================================
// runtime.inl
//========================================================================

#include "applrts-runtime.hpp"

namespace applrts {

namespace local {

SimpleDeque<Task*> g_taskq = SimpleDeque<Task*>();

} // namespace local

namespace global {

int g_stop_flag __attribute__ ((section (".dram"))) = 0;

} // namespace global

} // namespace applrts
