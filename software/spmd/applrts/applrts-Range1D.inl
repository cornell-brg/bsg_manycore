//========================================================================
// Range1D.inl
//========================================================================

namespace applrts {

//----------------------------------------------------------------------
// Range1D class
//----------------------------------------------------------------------

// Constructors
template <typename IterType>
Range1D<IterType>::Range1D()
{
}

template <typename IterType>
Range1D<IterType>::Range1D( IterType begin, IterType end, size_t grain )
    : m_end( end ), m_begin( begin ), m_grainsize( grain )
{
}

// Range1D can be used as an STL container

template <typename IterType>
IterType Range1D<IterType>::begin() const
{
  return m_begin;
}

template <typename IterType>
IterType Range1D<IterType>::end() const
{
  return m_end;
}

template <typename IterType>
size_t Range1D<IterType>::size() const
{
  return ( size_t )( m_end - m_begin );
}

template <typename IterType>
bool Range1D<IterType>::empty() const
{
  return m_begin >= m_end;
}

template <typename IterType>
size_t Range1D<IterType>::grainsize() const
{
  return m_grainsize;
}

template <typename IterType>
bool Range1D<IterType>::divisible() const
{
  return m_grainsize < size();
}

template <typename IterType>
Range1D<IterType> Range1D<IterType>::split( size_t p )
{
  IterType mid     = m_begin + ( m_end - m_begin ) / p;
  IterType new_end = m_end;
  m_end            = mid;
  return Range1D<IterType>( mid, new_end, m_grainsize );
}

//----------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------

template <typename IterType>
Range1D<IterType> mk_range1d( const IterType& begin, const IterType& end,
                              size_t grain )
{
  return Range1D<IterType>( begin, end, grain );
}

} // namespace applrts
