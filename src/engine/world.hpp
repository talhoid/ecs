#pragma once
#include "component.hpp"
#include "entity.hpp"
#include "system.hpp"
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Entities = std::unordered_set<Entity *>;

class World {
public:
  World() : next_id(0), next_bit(0) {}

  ~World() {
    for (auto entity : pool)
      delete entity;
    for (auto entity : despawned)
      delete entity;
  }

  Entity *spawn();
  void despawn(Entity *entity);

  template <typename T> int assign() {
    return assign(std::type_index(typeid(T)));
  }

  int assign(std::type_index type) {
    if (ids.find(type) == ids.end()) {
      ids[type] = next_bit++;
      return ids[type];
    }

    return ids[type];
  }

  void collect(Entity *entity, Archetype previous);
  void compile();

  void add(System *system);
  void remove(System *system);

  void update(float delta);
  void render(float alpha);

private:
  int next_id;
  int next_bit;
  std::unordered_map<std::type_index, int> ids;
  std::unordered_map<Archetype, Entities> archetypes;
  std::unordered_map<System *, Entities *> systems;
  std::unordered_map<System *, Entities *> ordered_systems;
  std::unordered_map<System *, Entities *> unordered_systems;
  std::unordered_map<System *, Entities *> render_systems;
  std::vector<std::pair<System *, Entities *>> sorted_systems;
  std::vector<Entity *> pool;
  std::vector<Entity *> despawned;

  void sort_systems();
  void sync(Entity *entity);
  void detach(Entity *entity, Archetype archetype);
  void detach(Entity *entity);
  void sync(Entity *entity, System *system, Entities *entities);
};