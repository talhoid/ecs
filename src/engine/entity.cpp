#include "entity.hpp"
#include "component.hpp"
#include "world.hpp"

Entity::Entity(int id, World *world) : id(id), world(world) {}

void Entity::despawn() { world->despawn(this); }

void Entity::dispose() {
  components.clear();
  mask.reset();
}
