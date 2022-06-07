//========================================================================
// parallel_for.inl
//========================================================================

#include "applrts-Range1D.hpp"
#include "applrts-scheduler.hpp"
#include "applrts-Task.hpp"
#include "applrts-config.hpp"

namespace applrts {

//----------------------------------------------------------------------
// Task definition
//----------------------------------------------------------------------
// templated class used by parallel_for

template <typename RangeT, typename BodyT>
class ParallelForTask : public Task {
public:
  ParallelForTask( const RangeT& range, const BodyT& body )
      : m_range( range ), m_body( body )
  {
  }

  Task* execute()
  {
    if ( m_range.divisible() ) {
      RangeT                    new_range = m_range.split();
      Task                      join_point( 2 );

      // moyang TO-DO: remove TBIW-related code for now
      // int level = get_current_level();

      ParallelForTask<RangeT, BodyT> right_half( new_range, m_body );
      right_half.set_successor( &join_point );

      // moyang TO-DO: remove TBIW-related code for now
      // right_half.set_level(level+1);
      // join_point.set_level(level);

      // spawn the right half
      spawn( &right_half );

      // moyang TO-DO: remove TBIW-related code for now
      // level = increase_current_level();
      // set_region(level);

      // execute the left half directly
      execute();

      // moyang TO-DO: remove TBIW-related code for now
      // set_region(level-1); // revert to pre-spawn level
      // int new_level = get_current_level();
      // set_current_level(level-1); // revert to pre-spawn level
      // join_point.update_level_max(new_level+1);

      wait( &join_point );

      // moyang TO-DO: remove TBIW-related code for now
      // level = join_point.get_level();
      // set_current_level(level);
      // set_region(level);
    }
    else {
      m_body( m_range );
    }
    return nullptr;
  }

private:
  RangeT m_range;
  BodyT  m_body;
};

//----------------------------------------------------------------------
// IndexRangeBody
//----------------------------------------------------------------------
// Functor that takes a range object

template <typename IndexT, typename BodyT>
class IndexRangeBody {
public:
  IndexRangeBody( const BodyT& body, IndexT& begin, IndexT& step )
      : m_body( body ), m_begin( begin ), m_step( step )
  {
  }

  void operator()( const Range1D<IndexT>& range ) const
  {
    IndexT b = range.begin();
    IndexT e = range.end();
    IndexT s = m_step;
    IndexT k = m_begin + b * s;
    for ( IndexT i = b; i < e; ++i, k += s ) {
      m_body( k );
    }
  }

private:
  const BodyT&  m_body;
  const IndexT& m_begin;
  const IndexT& m_step;
};

//----------------------------------------------------------------------
// Templated functions
//----------------------------------------------------------------------

template <typename RangeT, typename BodyT>
void parallel_for( const RangeT& range, const BodyT& body )
{
  Task                      dummy_join( 2 );
  ParallelForTask<RangeT, BodyT> root( range, body );

  // moyang TO-DO: remove TBIW-related code for now
  // int level = get_current_level();
  // dummy_join.set_level(level);
  // root.set_level(level+1);

  root.set_successor( &dummy_join );

  spawn( &root );
  wait( &dummy_join );

  // moyang TO-DO: remove TBIW-related code for now
  // level = dummy_join.get_level();
  // set_current_level(level);
  // set_region(level);
}

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body )
{
  if ( first < last ) {
    // use the app-specific grain size
    size_t grain = get_grain_size(first, last);

    IndexT end = ( last - first - IndexT( 1 ) ) / step + IndexT( 1 );
    Range1D<IndexT>               range( IndexT( 0 ), end, grain );
    IndexRangeBody<IndexT, BodyT> range_body( body, first, step );
    parallel_for( range, range_body );
  }
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body )
{
  if ( first < last ) {
    IndexT end = ( last - first - IndexT( 1 ) ) / step + IndexT( 1 );
    Range1D<IndexT>               range( IndexT( 0 ), end, 1 );
    IndexRangeBody<IndexT, BodyT> range_body( body, first, step );
    parallel_for( range, range_body );
  }
}

} // namespace applrts
