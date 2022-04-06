//========================================================================
// runtime.inl
//========================================================================

#include "appl-runtime.hpp"

namespace appl {

namespace global {

int g_stop_flag __attribute__ ((section (".dram"))) = 0;

} // namespace global

} // namespace appl
