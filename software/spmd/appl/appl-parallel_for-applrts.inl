//========================================================================
// parallel_for.inl
//========================================================================

#include "applrts.hpp"

namespace appl {

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body )
{
  applrts::parallel_for( first, last, step, body );
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body )
{
  applrts::parallel_for_1( first, last, step, body );
}

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, const BodyT& body )
{
  applrts::parallel_for( first, last, IndexT( 1 ), body );
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, const BodyT& body )
{
  applrts::parallel_for_1( first, last, IndexT( 1 ), body );
}

} // namespace appl
