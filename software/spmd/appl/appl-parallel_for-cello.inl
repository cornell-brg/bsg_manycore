//========================================================================
// parallel_for.inl
//========================================================================
#include "cello.hpp"
namespace appl {

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body )
{
    cello::foreach(first, last, step, body);
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body )
{
    cello::foreach(first, last, step, body);
}

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, const BodyT& body )
{
    cello::foreach(first, last, body);
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, const BodyT& body )
{
    cello::foreach(first, last, body);
}

} // namespace appl
