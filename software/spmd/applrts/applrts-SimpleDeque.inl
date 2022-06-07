//========================================================================
// SimpleDeque.inl
//========================================================================

namespace applrts {

template <typename T>
SimpleDeque<T>::SimpleDeque() {
}

template <typename T>
void SimpleDeque<T>::reset() {
  m_array_rp  = remote_ptr(m_array, bsg_x, bsg_y);
  m_head_ptr  = m_array_rp;
  m_tail_ptr  = m_array_rp;
  m_array_end = m_array_rp + QUEUE_SIZE;

  m_mutex_ptr = (bsg_mcs_mutex_t*)appl::appl_malloc(sizeof(bsg_mcs_mutex_t));
  bsg_print_hexadecimal((intptr_t)m_mutex_ptr);
  m_lcl_rp = remote_ptr(&m_lcl, bsg_x, bsg_y);
  bsg_print_int(12580);
  bsg_print_hexadecimal((intptr_t)m_lcl_rp);

  bsg_print_int((intptr_t)m_head_ptr);
  bsg_print_int((intptr_t)m_tail_ptr);
  bsg_print_int((intptr_t)m_array_end);
  bsg_print_int((intptr_t)m_array_rp);
}

template <typename T>
void SimpleDeque<T>::push_back( const T& item )
{
  lock();
  asm volatile("": : :"memory");
  if ( m_tail_ptr < m_array_end ) {
    *m_tail_ptr = item;
    m_tail_ptr++;
  }
  else {
    // we ran out of queue ... fatal error
    bsg_print_int(7700);

  }
  asm volatile("": : :"memory");
  unlock();
}

template <typename T>
T SimpleDeque<T>::pop_back()
{
  lock();
  asm volatile("": : :"memory");
  T ret_val = nullptr;
  if ( m_tail_ptr - m_head_ptr > 0 ) {
    T* tmp = --m_tail_ptr;
    // reset pointer
    if ( m_tail_ptr == m_head_ptr ) {
      m_tail_ptr = m_head_ptr = m_array_rp;
    }
    ret_val = *tmp;
  }
  asm volatile("": : :"memory");
  unlock();
  return ret_val;
}

template <typename T>
T SimpleDeque<T>::pop_front()
{
  lock();
  asm volatile("": : :"memory");
  T ret_val = nullptr;
  if ( m_tail_ptr - m_head_ptr > 0 ) {
    T* tmp = m_head_ptr++;
    // reset pointer
    if ( m_tail_ptr == m_head_ptr ) {
      m_tail_ptr = m_head_ptr = m_array_rp;
    }
    ret_val = *tmp;
  }
  asm volatile("": : :"memory");
  unlock();
  return ret_val;
}

} // namespace applrts
