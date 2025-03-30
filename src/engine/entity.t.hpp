#include "entity.hpp"
#include "world.hpp"

template <typename T, typename... Args,
          typename std::enable_if_t<std::is_base_of_v<Component, T> &&
                                    std::is_constructible_v<T, Args...>> *>
Entity &Entity::add(Args &&...args) {
  Archetype previous(this->archetype);
  components[get_type_id<T>()] =
      std::make_unique<T>(std::forward<Args>(args)...);
  archetype.set(world->assign<T>());
  world->collect(this, previous);
  return *this;
}

template <typename T,
          typename std::enable_if_t<std::is_base_of_v<Component, T>> *>
Entity &Entity::remove() {
  Archetype previous(this->archetype);
  components.erase(get_type_id<T>());
  archetype.reset(world->assign<T>());
  world->collect(this, previous);
  return *this;
}

template <typename T,
          typename std::enable_if_t<std::is_base_of_v<Component, T>> *>
T *Entity::get() {
  auto it = components.find(get_type_id<T>());
  if (it != components.end()) {
    return static_cast<T *>(it->second.get());
  }
  return nullptr;
}

template <typename T,
          typename std::enable_if_t<std::is_base_of_v<Component, T>> *>
bool Entity::has() {
  return components.find(get_type_id<T>()) != components.end();
}