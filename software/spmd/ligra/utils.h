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

template <class ET>
inline void writeMin(ET* p, ET val) {
  ET result;
  asm volatile ("amomin.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
}

template <class ET>
inline void writeMinu(ET* p, ET val) {
  ET result;
  asm volatile ("amominu.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
}

template <class ET>
inline void writeMax(ET* p, ET val) {
  ET result;
  asm volatile ("amomax.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
}

template <class ET>
inline void writeMaxu(ET* p, ET val) {
  ET result;
  asm volatile ("amomaxu.w %[result], %[val], 0(%[p])" \
                : [result] "=r" (result) \
                : [p] "r" (p), [val] "r" (val));
}

#endif
