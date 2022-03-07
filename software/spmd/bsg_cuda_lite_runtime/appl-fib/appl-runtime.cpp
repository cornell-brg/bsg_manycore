//========================================================================
// runtime.inl
//========================================================================

#include "appl-runtime.hpp"

namespace appl {

namespace local {

SimpleDeque<Task*>* g_taskq_p = nullptr;

} // namespace local

namespace global {

int g_stop_flag __attribute__ ((section (".dram"))) = 0;

} // namespace global

} // namespace appl
