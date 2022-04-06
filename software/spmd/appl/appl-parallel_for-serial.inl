//========================================================================
// parallel_for.inl
//========================================================================

namespace appl {

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, IndexT step,
                   const BodyT& body )
{
  for (IndexT i = first; i < last; i += step) {
    body( i );
  }
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, IndexT step,
                     const BodyT& body )
{
  for (IndexT i = first; i < last; i += step) {
    body( i );
  }
}

template <typename IndexT, typename BodyT>
void parallel_for( IndexT first, IndexT last, const BodyT& body )
{
  for (IndexT i = first; i < last; i += IndexT( 1 )) {
    body( i );
  }
}

template <typename IndexT, typename BodyT>
void parallel_for_1( IndexT first, IndexT last, const BodyT& body )
{
  for (IndexT i = first; i < last; i += IndexT( 1 )) {
    body( i );
  }
}

} // namespace appl
