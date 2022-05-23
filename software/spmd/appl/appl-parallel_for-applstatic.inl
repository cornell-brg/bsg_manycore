//========================================================================
// parallel_for.inl
//========================================================================

#include "applstatic.hpp"

namespace appl {

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body )
{
  applstatic::parallel_for( first, last, step, body );
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body )
{
  applstatic::parallel_for_1( first, last, step, body );
}

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, const BodyT& body )
{
  applstatic::parallel_for( first, last, IndexT( 1 ), body );
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, const BodyT& body )
{
  applstatic::parallel_for_1( first, last, IndexT( 1 ), body );
}

} // namespace appl
