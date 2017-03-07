#ifndef ECS_ENTITY_COLLECTION_H
#define ECS_ENTITY_COLLECTION_H
#include "Handler.h"
#include "VariableComponent.h"
#include "extended_type_traits.h"
#include <exception>
#include <iostream>
#include <stddef.h>
#include <tuple>
namespace ecs {
/// EntityCollectionInner is an EntityCollection which deals only in pure
/// Entity<...> objects. Because these will be used as basetypes for the user
/// defined entities this helps deal type conflicts.
/// To be more precise: the Entity class has a reference to it's container. It
/// is unaware of the whatever type the user wrapped the Entity in  (e.g. Track
/// : Entity<...>). The collection is aware. While it could be made unaware by
/// having handler return an oblivious Collection This would make a
/// EntityCollectin<Vertex<X>>  the same as a EntityCollection<Track<X>>, this
/// workaround maintains a clear distinction of types and lets the user directly
/// use typedefs for their entites in declaring variables.
///
template <typename Entity> class EntityCollectionInner {
protected:
  /// how many entities there are
  uint64_t mCount;
  /// data pointer for each component of the entity. Direct Component* for basic
  /// components, special structure for N->M mapped components (array of index,
  /// size pairs for each entity followed by the array of component values)
  void *mComponentCollections[Entity::Size];
  EntityCollectionInner<Entity>(){};

public:
  // Get a pointer to the component array based on type.
  template <
      typename C, int index = Entity::template IndexOf<C>(),
      typename std::enable_if<!std::is_base_of<IVariableComponent, C>::value &&
                              (index >= 0)>::type * = nullptr>
  constexpr C *get() const {
    return (C *)mComponentCollections[index];
  }
  // Same but for N->M, returns a pair of pointers. The first one points to an
  // array of <index, size> and the second points to the component data. The
  // index is into the second array.
  template <
      typename C, int index = Entity::template IndexOf<C>(),
      typename std::enable_if<std::is_base_of<IVariableComponent, C>::value &&
                              (index >= 0)>::type * = nullptr>
  std::pair<uint64_t *, C *> get() const {
    return std::make_pair(
        (uint64_t *)mComponentCollections[index],
        (C *)((uint64_t *)mComponentCollections[index] + 2 * mCount));
  }
  size_t size() const { return mCount; }
};

/// The 'real' EntityCollection.
template <typename Entity>
class EntityCollection : public EntityCollectionInner<typename Entity::Self> {
private:
  /// fetches the data pointers from a given handler. probably better to from
  /// handler instead.
  template <unsigned N = 0,
            typename std::enable_if<N<Entity::Size>::type * = nullptr> void
                internal_tryGetHandler(Handler &handler) {
    this->mComponentCollections[N] =
        (void *)handler
            .template getComponentData<
                Entity, typename Entity::template TypeOf<N>::type>()
            .data();
    internal_tryGetHandler<N + 1>(handler);
  }
  /// termination specialization
  template <unsigned N,
            typename std::enable_if<N == Entity::Size>::type * = nullptr>
  void internal_tryGetHandler(Handler &handler) {}

public:
  /// Construct a collection from a handler reference, grabbing the data
  /// pointers from there.
  EntityCollection<Entity>(Handler &handler) {
    this->mCount = handler.getCount<Entity>();
    internal_tryGetHandler<0>(handler);
  }
  // returns an Entity i.e. (index, reference) pair
  Entity operator[](size_t index) { return Entity(this, index); }
};
}
#endif