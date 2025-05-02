#include "entity.hpp"
#include "component.hpp"
#include "world.hpp"
#include "logger.hpp"

Entity::Entity(int id, World* world) 
    : id(id), mask(0), archetype(nullptr), index(0), world(world) {
    LOG_DEBUG("Created entity ", id);
}

void Entity::despawn() {
    LOG_DEBUG("Despawning entity ", id);
    world->despawn(this);
}

void Entity::dispose() {
    LOG_DEBUG("Disposing entity ", id);
    world = nullptr;
    mask.reset();
    archetype = nullptr;
    index = 0;
}
