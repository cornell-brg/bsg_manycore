//========================================================================
// Task.inl
//========================================================================

namespace appl {

inline Task::Task()
    : m_ready_count( 0 ), m_successor_ptr( nullptr ),
{
}

inline Task::Task( int ready_count )
    : m_ready_count( ready_count ), m_successor_ptr( nullptr ),
{
}

inline Task::Task( int ready_count, Task* succ_p )
    : m_ready_count( ready_count ), m_successor_ptr( succ_p ),
{
  if ( succ_p )
    succ_p->increment_ready_count();
}

inline Task::Task( Task&& t )
{
  m_successor_ptr = t.m_successor_ptr;
  m_destroy_flag = t.m_destroy_flag;
  // XXX: handle m_ready_count
}

inline Task* Task::execute()
{
  return nullptr;
}

inline int Task::get_ready_count()
{
}

inline void Task::set_ready_count( int ready_count )
{
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
}

inline int Task::increment_ready_count()
{
}

} // namespace appl
