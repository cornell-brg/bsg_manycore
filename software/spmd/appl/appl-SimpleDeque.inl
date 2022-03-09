//========================================================================
// SimpleDeque.inl
//========================================================================

namespace appl {

template <typename T>
SimpleDeque<T>::SimpleDeque() {
  m_head_ptr  = m_array;
  m_tail_ptr  = m_array;
  m_array_end = m_array + QUEUE_SIZE;

  m_mutex_ptr = brg_malloc();
  m_size_ptr  = brg_malloc();

  // this does not order anything
  bsg_amoswap(m_mutex_ptr, 0);
  bsg_amoswap(m_size_ptr, 0);
}

template <typename T>
void SimpleDeque<T>::push_back( const T& item )
{
  lock( m_mutex_ptr );
  if ( m_tail_ptr < m_array_end ) {
    *m_tail_ptr = item;
    m_tail_ptr++;
    bsg_amoadd(m_size_ptr, 1);
  }
  else {
    // we ran out of queue ... fatal error
    bsg_print_int(7700);
  }
  unlock( m_mutex_ptr );
}

template <typename T>
T SimpleDeque<T>::pop_back()
{
  lock( m_mutex_ptr );
  if ( m_tail_ptr - m_head_ptr > 0 ) {
    T* tmp = --m_tail_ptr;
    // reset pointer
    if ( m_tail_ptr == m_head_ptr ) {
      m_tail_ptr = m_head_ptr = m_array;
    }
    bsg_amoadd(m_size_ptr, -1);
    unlock( m_mutex_ptr );
    return *tmp;
  }
  else {
    unlock( m_mutex_ptr );
    return nullptr;
  }
}

template <typename T>
T SimpleDeque<T>::pop_front()
{
  lock( m_mutex_ptr );
  if ( m_tail_ptr - m_head_ptr > 0 ) {
    T* tmp = m_head_ptr++;
    // reset pointer
    if ( m_tail_ptr == m_head_ptr ) {
      m_tail_ptr = m_head_ptr = m_array;
    }
    bsg_amoadd(m_size_ptr, -1);
    unlock( m_mutex_ptr );
    return *tmp;
  }
  else {
    unlock( m_mutex_ptr );
    return nullptr;
  }
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
