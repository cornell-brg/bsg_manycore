//========================================================================
// parallel_reduce.inl
//========================================================================

#include "applrts-Range1D.hpp"
#include "applrts-scheduler.hpp"
#include "applrts-Task.hpp"
#include "applrts-config.hpp"

namespace applrts {

//----------------------------------------------------------------------
// ParallelReduceTask
//----------------------------------------------------------------------
// templated class used by parallel_reduce

template <typename RangeT, typename BodyT>
class ParallelReduceTask : public Task {
public:
  ParallelReduceTask( const RangeT& range, const BodyT& body )
      : m_range( range ), m_body( body )
  {
    // m_size = sizeof(ParallelReduceTask<RangeT, BodyT>);
    m_size = -1;
  }

  ParallelReduceTask<RangeT, BodyT> split()
  {
    return ParallelReduceTask<RangeT, BodyT>( m_range.split(),
                                              m_body.split() );
  }

  Task* execute()
  {
    if ( m_range.divisible() ) {
      Task join_point( 2 );
      auto right_half = this->split();

      right_half.set_successor( &join_point );

      // spawn the right half
      spawn( &right_half );

      // execute the left half directly
      execute();

      wait( &join_point );

      // reduce
      m_body.reduce( right_half.m_body );
    }
    else {
      m_body( m_range );
    }
    return nullptr;
  }

  BodyT get_body() const { return m_body; }

private:
  RangeT      m_range;
  BodyT       m_body;
};

//----------------------------------------------------------------------
// ParallelReducer
//----------------------------------------------------------------------
// Reduce object for functional form of parallel_reduce

template <typename RangeT, typename ValueT, typename FuncT,
          typename ReduceT>
class ParallelReducer {
public:
  ValueT value() const { return m_value; }

  ParallelReducer( const ValueT& initV, const ValueT& value,
                   const FuncT& func, const ReduceT& reduce )
      : m_initV( initV ), m_value( value ), m_func( func ),
        m_reduce( reduce )
  {
  }

  ParallelReducer<RangeT, ValueT, FuncT, ReduceT> split()
  {
    return ParallelReducer<RangeT, ValueT, FuncT, ReduceT>(
        m_initV, m_initV, m_func, m_reduce );
  }

  void
  reduce( const ParallelReducer<RangeT, ValueT, FuncT, ReduceT>& rhs )
  {
    m_value = m_reduce( m_value, rhs.m_value );
  }

  void operator()( const RangeT& range )
  {
    m_value = m_func( range.begin(), range.end(), m_initV );
  }

  ValueT         m_value;
private:
  ValueT         m_initV;
  const FuncT    m_func;
  const ReduceT  m_reduce;
};

//----------------------------------------------------------------------
// Templated functions
//----------------------------------------------------------------------

// Imperative form
template <typename RangeT, typename BodyT>
void parallel_reduce( const RangeT& range, BodyT& body )
{
  Task  dummy_join( 2 );
  ParallelReduceTask<RangeT, BodyT> root( range, body );

  root.set_successor( &dummy_join );

  spawn( &root );
  wait( &dummy_join );

  // body = BodyT( root.get_body() );
  body.m_value = BodyT( root.get_body() ).m_value;
}

// Functional form
template <typename RangeT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( const RangeT& range, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce )
{
  ParallelReducer<RangeT, ValueT, FuncT, ReduceT>
    reducer( initV, initV, func, reduce );
  parallel_reduce( range, reducer );
  return reducer.value();
}

template <typename IndexT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( IndexT first, IndexT last, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce )
{
  if ( first < last ) {
    // use the app-specific grain size
    size_t grain = get_grain_size(first, last);

    IndexT end = ( last - first );
    Range1D<IndexT> range( IndexT( 0 ), end, grain );
    return parallel_reduce(range, initV, func, reduce);
  } else {
    return ValueT(-1);
  }
}

} // namespace applrts
