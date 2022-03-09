//========================================================================
// Range1D.h
//========================================================================
// One-dimensional range object, highly inspired by Intel TBB
// blocked_range.
//

#ifndef APPL_RANGE_1D_H
#define APPL_RANGE_1D_H

namespace appl {

//----------------------------------------------------------------------
// Range1D class
//----------------------------------------------------------------------

template <typename IterType>
class Range1D {
public:
  // Constructors

  Range1D();

  Range1D( IterType begin, IterType end, size_t grain = 1 );

  // Range1D can be used as an STL container

  using const_iterator = IterType;

  const_iterator begin() const;

  const_iterator end() const;

  size_t size() const;

  bool empty() const;

  // Split interface: a range object can be splited into smaller
  // ranges, up to the grainsize

  size_t grainsize() const;

  bool divisible() const;

  // Return a new range that is the first 1/p of the original range,
  // this object holds remaining (p-1)/p.
  Range1D<IterType> split( size_t p = 2 );

private:
  IterType m_end;
  IterType m_begin;
  size_t   m_grainsize;
};

//----------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------

template <typename IterType>
Range1D<IterType> mk_range1d( const IterType& begin, const IterType& end,
                              size_t grain = 2 );

} // namespace appl

#include "appl-Range1D.inl"

#endif
