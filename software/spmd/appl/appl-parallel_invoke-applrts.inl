//========================================================================
// parallel_invoke.inl
//========================================================================

#include "applrts.hpp"

namespace appl {

// Spawn 2, 3, 4, ... functors in parallel

template <typename Func0, typename Func1>
void parallel_invoke( const Func0& func0, const Func1& func1 )
{
  applrts::parallel_invoke( func0, func1 );
}

/*
template <typename Func0, typename Func1, typename Func2>
void parallel_invoke( const Func0& func0, const Func1& func1,
                      const Func2& func2 )
{
  applrts::parallel_invoke( func0, func1, func2 );
}
*/

template <typename Func0, typename Func1, typename Func2, typename Func3>
void parallel_invoke( const Func0& func0, const Func1& func1,
                      const Func2& func2, const Func3& func3 )
{
  applrts::parallel_invoke( func0, func1, func2, func3 );
}

/*
template <typename Func0, typename Func1, typename Func2, typename Func3,
          typename Func4,  typename Func5,  typename Func6>
void parallel_invoke( const Func0& func0, const Func1& func1,
                      const Func2& func2, const Func3& func3,
                      const Func4& func4, const Func5& func5,
                      const Func6& func6)
{
  applrts::parallel_invoke( func0, func1, func2, func3, func4,
                            func5, func6 );
}
*/

} // namespace appl
