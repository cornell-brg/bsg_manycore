//========================================================================
// parallel_for.inl
//========================================================================
#include "limoncello.hpp"
namespace appl {

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body )
{
    cello::for_each(first, last, step, body);
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body )
{
    cello::for_each(first, last, step, body);
}

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, const BodyT& body )
{
    cello::for_each(first, last, body);
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, const BodyT& body )
{
    cello::for_each(first, last, body);
}

} // namespace appl