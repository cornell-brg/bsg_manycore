//========================================================================
// SimpleDeque.h
//========================================================================
// Simple deque protected by simple lock.

#ifndef APPLRTS_SIMPLEDEQUE_H
#define APPLRTS_SIMPLEDEQUE_H

#include <cstddef>
#include "bsg_manycore_atomic.h"
#include "bsg_set_tile_x_y.h"
#include "applrts-config.hpp"

#define QUEUE_SIZE 160

namespace applrts {

inline void lock(int* lock_ptr) {
  int lock_val = 1;
  do {
    lock_val = bsg_amoswap_aq(lock_ptr, 1);
  } while (lock_val != 0);
  return;
}

inline void unlock(int* lock_ptr) {
  bsg_amoswap_rl(lock_ptr, 0);
}

template <typename T>
class SimpleDeque {
public:
  SimpleDeque();
  ~SimpleDeque() {};
  void reset();
  void push_back( const T& item );
  T pop_back();
  T pop_front();

private:
  T* m_array_rp; // this is a pointer to m_array in remote format
  T* m_head_ptr;
  T* m_tail_ptr;
  T* m_array_end;

  int* m_mutex_ptr;

  T  m_array[QUEUE_SIZE];

};

} // namespace applrts

#include "applrts-SimpleDeque.inl"

#endif
