#ifndef UTILS_H
#define UTILS_H

#include "appl.hpp"

#if defined(APPL_IMPL_CELLO)
#define newA( __E, __n ) (__E*)cello::arch::malloc( ( __n ) * sizeof( __E ) )
#else
#define newA( __E, __n ) (__E*)appl::appl_malloc( ( __n ) * sizeof( __E ) )
#endif

template <class ET>
inline void writeAdd(ET* p, ET val) {
  ET result;
  asm volatile ("amoadd.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
}

template <class ET>
inline bool writeMin(ET* p, ET val) {
  ET result;
  asm volatile ("amomin.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
  return result > val;
}

template <class ET>
inline bool writeMinu(ET* p, ET val) {
  ET result;
  asm volatile ("amominu.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
  return result > val;
}

template <class ET>
inline ET min(ET a, ET b) {
  if (a > b) {
    return b;
  } else {
    return a;
  }
}

#endif
