#ifndef APPLRTS_PARALLEL_FOR_H
#define APPLRTS_PARALLEL_FOR_H

namespace applrts {

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body );

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body );

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, const BodyT& body );

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, const BodyT& body );

} // namespace applrts

#include "applrts-parallel_for.inl"

#endif
