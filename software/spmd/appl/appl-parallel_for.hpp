#ifndef APPL_PARALLEL_FOR_H
#define APPL_PARALLEL_FOR_H

namespace appl {

/*
template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body );
*/

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body );

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, const BodyT& body );

/*
template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, const BodyT& body );
*/

} // namespace appl

#include "appl-parallel_for.inl"

#endif
