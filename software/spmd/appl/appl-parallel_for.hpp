#ifndef APPL_PARALLEL_FOR_H
#define APPL_PARALLEL_FOR_H

#include "appl-config.hpp"

namespace appl {

//----------------------------------------------------------------------
// parallel_for for a body that takes single iteration
//----------------------------------------------------------------------
// The body is evaluated from _begin_ to _end_ with a stride of
// _step_. Note that unlike the previous function, the body should only
// take a single iterator. Here is an example:
//
// size_t N = 1000, step = 4;
// appl::parallel_for( 0, size_t(N), step, [=] ( size_t i ) {
//     // do work on i-th iteration
//   } );
//
// equal to serial code:
//
// for( auto i = first; i < last; i += step)
//    body(i);
//
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

} // namespace appl

#ifdef APPL_IMPL_APPLRTS
#include "appl-parallel_for-applrts.inl"
#elif defined(APPL_IMPL_CELLO)
#include "appl-parallel_for-cello.inl"
#elif defined(APPL_IMPL_STATIC)
#include "appl-parallel_for-applstatic.inl"
#else
#include "appl-parallel_for-serial.inl"
#endif

#endif
