//========================================================================
// FunctorTask.cpp
//========================================================================

namespace applrts {

template <typename Func>
FunctorTask<Func>::FunctorTask( Func&& func )
    : Task( 0 ), m_func( std::forward<Func>( func ) )
{
  m_size = sizeof(FunctorTask<Func>);
}

template <typename Func>
FunctorTask<Func>::FunctorTask( Func&& func, Task* succ_p )
    : Task( 0, succ_p ), m_func( std::forward<Func>( func ) )
{
  m_size = sizeof(FunctorTask<Func>);
}

template <typename Func>
FunctorTask<Func>::FunctorTask( FunctorTask&& t )
    : Task( t.get_ready_count(), t.get_successor() ),
      m_func( std::forward<Func>( t.m_func ) )
{
  m_size = sizeof(FunctorTask<Func>);
}

template <typename Func>
Task* FunctorTask<Func>::execute()
{
  m_func();
  return nullptr;
}

} // namespace applrts

