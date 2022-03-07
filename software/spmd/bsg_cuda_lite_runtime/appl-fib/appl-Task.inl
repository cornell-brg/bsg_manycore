//========================================================================
// Task.inl
//========================================================================

namespace appl {

inline Task::Task()
    : m_successor_ptr( nullptr )
{
  m_ready_count_ptr = brg_malloc();
  set_ready_count( 0 );
}

inline Task::Task( int ready_count )
    : m_successor_ptr( nullptr )
{
  m_ready_count_ptr = brg_malloc();
  set_ready_count( ready_count );
}

inline Task::Task( int ready_count, Task* succ_p )
    : m_successor_ptr( succ_p )
{
  m_ready_count_ptr = brg_malloc();
  set_ready_count( ready_count );
  if ( succ_p )
    succ_p->increment_ready_count();
}

inline Task::Task( Task&& t )
{
  m_successor_ptr = t.m_successor_ptr;
  m_ready_count_ptr = t.m_ready_count_ptr;
}

/*
inline Task::~Task() {
  // remove an active task from ref_count_stack
  // XXX: disable for now -- it may not have stack behavior
  // ref_count_stack_idx--;
}
*/

inline Task* Task::execute()
{
  return nullptr;
}

inline int Task::get_ready_count()
{
  // memory order relaxed
  return bsg_amoadd(m_ready_count_ptr, 0);
}

inline void Task::set_ready_count( int ready_count )
{
  // this one has to be SC? I'm not certain.
  // need to see where this set_ready_count is called
  bsg_amoswap_aqrl(m_ready_count_ptr, ready_count);
}

inline void Task::set_successor( Task* task_p )
{
  m_successor_ptr = task_p;
}

inline Task* Task::get_successor() const
{
  return m_successor_ptr;
}

inline int Task::decrement_ready_count()
{
  // XXX: use SC for now. I'm reason about it later
  return bsg_amoadd_aqrl(m_ready_count_ptr, -1);
}

inline int Task::increment_ready_count()
{
  // XXX: use SC for now. I'm reason about it later
  return bsg_amoadd_aqrl(m_ready_count_ptr, 1);
}

} // namespace appl
