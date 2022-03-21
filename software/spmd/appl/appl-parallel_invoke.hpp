//========================================================================
// parallel_invoke.h
//========================================================================

#ifndef APPL_PARALLEL_INVOKE_H
#define APPL_PARALLEL_INVOKE_H

namespace appl {

// Spawn 2, 3, 4, ... functors in parallel

template <typename Func0, typename Func1>
void parallel_invoke( const Func0& func0, const Func1& func1 );

/*
template <typename Func0, typename Func1, typename Func2>
void parallel_invoke( const Func0& func0, const Func1& func1,
                      const Func2& func2 );
*/

template <typename Func0, typename Func1, typename Func2, typename Func3>
void parallel_invoke( const Func0& func0, const Func1& func1,
                      const Func2& func2, const Func3& func3 );

/*
template <typename Func0, typename Func1, typename Func2, typename Func3,
          typename Func4,  typename Func5,  typename Func6>
void parallel_invoke( const Func0& func0, const Func1& func1,
                      const Func2& func2, const Func3& func3,
                      const Func4& func4, const Func5& func5,
                      const Func6& func6);
*/

} // namespace appl

#include "appl-parallel_invoke.inl"

#endif
