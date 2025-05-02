#pragma once
#include "entity.hpp"
#include "world.hpp"
#include "archetypes.hpp"

template<typename T, typename... Args>
Entity& Entity::add(Args&&... args) {
    ComponentMask new_mask = mask;
    new_mask.set(world->assign<T>());
    
    T component(std::forward<Args>(args)...);
    Archetype* new_archetype = world->get_or_create_archetype(new_mask);
    new_archetype->add_component_array<T>();  // Ensure array exists
    world->move_to_archetype(this, new_mask, &component);
    
    return *this;
}

template<typename T>
Entity& Entity::remove() {
    ComponentMask new_mask = mask;
    new_mask.reset(world->assign<T>());
    world->move_to_archetype(this, new_mask, nullptr);
    return *this;
}

template<typename T>
T* Entity::get() {
    if (!archetype) return nullptr;
    T* array = archetype->get_component_array<T>();
    if (!array) return nullptr;
    return &array[index];
}

template<typename T>
bool Entity::has() {
    return mask.test(world->assign<T>());
}
