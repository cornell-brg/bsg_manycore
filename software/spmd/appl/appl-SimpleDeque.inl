//========================================================================
// SimpleDeque.inl
//========================================================================

namespace appl {

template <typename T>
SimpleDeque<T>::SimpleDeque() {
}

template <typename T>
void SimpleDeque<T>::reset() {
  m_array_rp  = remote_ptr(m_array, bsg_x, bsg_y);
  m_head_ptr  = m_array_rp;
  m_tail_ptr  = m_array_rp;
  m_array_end = m_array_rp + QUEUE_SIZE;

  m_mutex_ptr = remote_ptr((int*)0, bsg_x, bsg_y);;
  bsg_print_hexadecimal((intptr_t)m_mutex_ptr);
  m_size_ptr  = brg_malloc();

  // this does not order anything
  bsg_amoswap(m_mutex_ptr, 0);
  bsg_amoswap(m_size_ptr, 0);

  bsg_print_int((intptr_t)m_head_ptr);
  bsg_print_int((intptr_t)m_tail_ptr);
  bsg_print_int((intptr_t)m_array_end);
  bsg_print_int((intptr_t)m_array_rp);
}

template <typename T>
void SimpleDeque<T>::push_back( const T& item )
{
  lock( m_mutex_ptr );
  asm volatile("": : :"memory");
  if ( m_tail_ptr < m_array_end ) {
    *m_tail_ptr = item;
    m_tail_ptr++;
    bsg_amoadd(m_size_ptr, 1);
  }
  else {
    // we ran out of queue ... fatal error
    bsg_print_int(7700);

  }
  asm volatile("": : :"memory");
  unlock( m_mutex_ptr );
}

template <typename T>
T SimpleDeque<T>::pop_back()
{
  lock( m_mutex_ptr );
  asm volatile("": : :"memory");
  T ret_val = nullptr;
  if ( m_tail_ptr - m_head_ptr > 0 ) {
    T* tmp = --m_tail_ptr;
    // reset pointer
    if ( m_tail_ptr == m_head_ptr ) {
      m_tail_ptr = m_head_ptr = m_array_rp;
    }
    bsg_amoadd(m_size_ptr, -1);
    ret_val = *tmp;
  }
  asm volatile("": : :"memory");
  unlock( m_mutex_ptr );
  return ret_val;
}

template <typename T>
T SimpleDeque<T>::pop_front()
{
  lock( m_mutex_ptr );
  asm volatile("": : :"memory");
  T ret_val = nullptr;
  if ( m_tail_ptr - m_head_ptr > 0 ) {
    T* tmp = m_head_ptr++;
    // reset pointer
    if ( m_tail_ptr == m_head_ptr ) {
      m_tail_ptr = m_head_ptr = m_array_rp;
    }
    bsg_amoadd(m_size_ptr, -1);
    ret_val = *tmp;
  }
  asm volatile("": : :"memory");
  unlock( m_mutex_ptr );
  return ret_val;
}

template <typename T>
size_t SimpleDeque<T>::size() const
{
  return bsg_amoadd(m_size_ptr, 0);
}

template <typename T>
bool SimpleDeque<T>::empty() const
{
  return size() == 0;
}

} // namespace appl
