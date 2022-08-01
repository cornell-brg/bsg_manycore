//========================================================================
// runtime.inl
//========================================================================

#include "applrts-runtime.hpp"

namespace applrts {

namespace local {

SimpleDeque<Task*>* g_taskq = nullptr;

} // namespace local

} // namespace applrts
