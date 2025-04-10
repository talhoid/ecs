#include "world.hpp"
#include "entity.hpp"
#include "system.hpp"
#include <functional>
#include <stdexcept>
#include <typeindex>
#include <utility>

Entity *World::spawn() {
  Entity *entity = pool.empty() ? new Entity(++next_id, this) : pool.back();
  if (!pool.empty())
    pool.pop_back();
  archetypes[entity->archetype].insert(entity);
  return entity;
}

void World::despawn(Entity *entity) { despawned.push_back(entity); }

void World::detach(Entity *entity, Archetype archetype) {
  auto &group = archetypes[archetype];
  group.erase(entity);
  if (group.empty())
    archetypes.erase(archetype);

  for (auto [system, entities] : systems) {
    entities->erase(entity);
    system->cleanup(entity);
  }
};
void World::detach(Entity *entity) { detach(entity, entity->archetype); };

void World::collect(Entity *entity, Archetype previous) {
  detach(entity);
  archetypes[entity->archetype].insert(entity);
  sync(entity);
}

void World::add(System *system) {
  system->compile(this);
  Entities *entities = new Entities();
  switch (system->mode) {
  case System::Mode::Ordered:
    ordered_systems.insert({system, entities});
    break;
  case System::Mode::Unordered:
    unordered_systems.insert({system, entities});
    break;
  case System::Mode::Render:
    render_systems.insert({system, entities});
    break;
  }
  systems.insert({system, entities});

  for (auto &[_, entities_] : archetypes) {
    for (auto &entity : entities_) {
      sync(entity, system, entities);
    }
  }
}

void World::remove(System *system) {
  ordered_systems.erase(system);
  unordered_systems.erase(system);
  render_systems.erase(system);
  systems.erase(system);
}

void World::compile() {

  for (auto &[system, entities] : ordered_systems) {
    for (auto &required : system->requires) {
      for (auto &[candidate, candidate_entities] : unordered_systems) {
        if (std::type_index(typeid(*candidate)) == required) {
          candidate->mode = System::Mode::Ordered;
          ordered_systems.insert({candidate, candidate_entities});
          unordered_systems.erase(candidate);
        }
      }
    }
  }

  sort_systems();
}

void World::sync(Entity *entity, System *system, Entities *entities) {
  if (system->query.match(&(entity->archetype))) {
    entities->insert(entity);
  } else {
    entities->erase(entity);
  }
}

void World::sync(Entity *entity) {
  for (auto &[system, entities] : systems) {
    sync(entity, system, entities);
  }
}

void World::update(float delta) {
  for (Entity *entity : despawned) {
    detach(entity);
    entity->dispose();
    pool.push_back(entity);
  }
  despawned.clear();

  for (const auto &[system, entities] : sorted_systems) {
    for (int i = 0; i < system->subticks; i++) {
      system->execute(*entities, delta / system->subticks, this);
    }
  }

  for (const auto &[system, entities] : unordered_systems) {
    for (int i = 0; i < system->subticks; i++) {
      system->execute(*entities, delta / system->subticks, this);
    }
  }
}

void World::render(float alpha) {
  for (const auto &[system, entities] : render_systems) {
    system->execute(*entities, alpha, this);
  }
}

void World::sort_systems() {
  std::unordered_map<System *, std::vector<System *>> graph;

  for (auto &[system, entities] : ordered_systems) {
    graph[system] = {};
  }

  for (auto &[system, entities] : ordered_systems) {
    for (auto &required : system->requires) {
      for (auto &[candidate, _] : ordered_systems) {
        if (std::type_index(typeid(*candidate)) == required) {
          graph[system].push_back(candidate);
        }
      }
    }
  }

  std::vector<std::pair<System *, Entities *>> sorted;
  std::unordered_map<System *, bool> visited;
  std::unordered_map<System *, bool> temp;

  std::function<void(System *)> visit = [&](System *system) {
    if (temp[system]) {
      throw std::runtime_error("Circular dependency in ordered systems");
    }
    if (!visited[system]) {
      temp[system] = true;
      for (auto &dep : graph[system]) {
        visit(dep);
      }
      visited[system] = true;
      temp[system] = false;
      sorted.push_back({system, ordered_systems[system]});
    }
  };

  for (auto &[system, entities] : ordered_systems) {
    if (!visited[system]) {
      visit(system);
    }
  }

  sorted_systems = sorted;
}