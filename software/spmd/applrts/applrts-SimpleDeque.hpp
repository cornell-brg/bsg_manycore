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
#include "appl-malloc.hpp"
#include "bsg_mcs_mutex.h"

#define QUEUE_SIZE 160

namespace applrts {

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

  bsg_mcs_mutex_t* m_mutex_ptr;
  bsg_mcs_mutex_node_t m_lcl;

  T  m_array[QUEUE_SIZE];

  // lcl_rp has to be the thread local one.
  inline void lock() {
    bsg_mcs_mutex_acquire(m_mutex_ptr, local::lcl_rp, local::lcl_rp);
  }
  inline void unlock() {
    bsg_mcs_mutex_release(m_mutex_ptr, local::lcl_rp, local::lcl_rp);
  }
};

} // namespace applrts

#include "applrts-SimpleDeque.inl"

#endif
