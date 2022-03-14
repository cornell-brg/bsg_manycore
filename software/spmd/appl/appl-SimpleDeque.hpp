//========================================================================
// SimpleDeque.h
//========================================================================
// Simple deque protected by simple lock.

#ifndef APPL_SIMPLEDEQUE_H
#define APPL_SIMPLEDEQUE_H

#include <cstddef>
#include "bsg_manycore_atomic.h"
#include "bsg_set_tile_x_y.h"
#include "appl-config.hpp"

#define QUEUE_SIZE 32

namespace appl {

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
  size_t size() const;
  bool empty() const;

private:
  T  m_array[QUEUE_SIZE];
  T* m_array_rp; // this is a pointer to m_array in remote format
  T* m_head_ptr;
  T* m_tail_ptr;
  T* m_array_end;

  // these 2 needs to be in the DRAM for AMO access
  int* m_mutex_ptr;
  int* m_size_ptr;

};

} // namespace appl

#include "appl-SimpleDeque.inl"

#endif
