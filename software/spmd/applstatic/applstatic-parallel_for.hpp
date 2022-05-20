#ifndef APPLSTATIC_PARALLEL_FOR_H
#define APPLSTATIC_PARALLEL_FOR_H

namespace applstatic {

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body );

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body );

} // namespace applstatic

#include "applstatic-parallel_for.inl"

#endif
