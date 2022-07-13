//========================================================================
// parallel_invoke.inl
//========================================================================

namespace appl {

// Spawn 2, 3, 4, ... functors in parallel

template <typename Func0, typename Func1>
void parallel_invoke( const Func0& func0, const Func1& func1 )
{
  func0();
  func1();
}

template <typename Func0, typename Func1, typename Func2, typename Func3>
void parallel_invoke( const Func0& func0, const Func1& func1,
                      const Func2& func2, const Func3& func3 )
{
  func0();
  func1();
  func2();
  func3();
}

} // namespace appl
