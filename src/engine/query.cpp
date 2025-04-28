#include "query.hpp"
#include "component.hpp"
#include "world.hpp"

void Query::compile(World *world) {
  for (auto requisite : requires) {
    required.set(world->assign(requisite));
  }
  for (auto rejection : rejects) {
    rejected.set(world->assign(rejection));
  }
}

bool Query::match(const ComponentMask *archetype) {
  return (((*archetype & required) == required) &&
          ((*archetype & rejected) == 0));
}

Entities Query::execute(std::unordered_map<ComponentMask, Entities> archetypes) {
  Entities matched;
  for (auto &[archetype, entities] : archetypes) {
    if (match(&archetype)) {
      matched.insert(entities.begin(), entities.end());
    }
  }
  return matched;
}