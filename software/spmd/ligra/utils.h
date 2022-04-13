#ifndef UTILS_H
#define UTILS_H

#include "appl.hpp"

#define newA( __E, __n ) (__E*)applrts::brg_malloc( ( __n ) * sizeof( __E ) )

template <class ET>
inline void writeAdd(ET* p, ET val) {
  ET result;
  asm volatile ("amoadd.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
}

#endif
