#pragma once
#include "component.hpp"
#include "entity.hpp"
#include <typeindex>
#include <unordered_set>

using Entities = std::unordered_set<Entity *>;
class Query {
public:
  template <typename T, typename std::enable_if_t<
                            std::is_base_of_v<Component, T>> * = nullptr>
  Query &has() {
    requires.insert(get_type_id<T>());
    return *this;
  }

  template <typename T, typename std::enable_if_t<
                            std::is_base_of_v<Component, T>> * = nullptr>
  Query &without() {
    rejects.insert(get_type_id<T>());
    return *this;
  }

  void compile(World *world);
  bool match(const Archetype *archetype);

  Entities execute(std::unordered_map<Archetype, Entities> archetypes);

private:
  Archetype required = 0;
  Archetype rejected = 0;
  std::unordered_set<std::type_index>
    requires;
  std::unordered_set<std::type_index> rejects;
};