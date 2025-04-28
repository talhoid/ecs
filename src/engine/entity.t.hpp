#pragma once
#include "entity.hpp"
#include "world.hpp"

template <typename T, typename... Args,
          typename std::enable_if_t<std::is_constructible_v<T, Args...>> *>
Entity &Entity::add(Args&&... args) {
    ComponentMask previous(this->archetype);
    components[get_type_id<T>()] = std::make_any<T>(std::forward<Args>(args)...);
    archetype.set(world->assign<T>());
    world->collect(this, previous);
    return *this;
}

template <typename T>
Entity &Entity::remove() {
    ComponentMask previous(this->archetype);
    components.erase(get_type_id<T>());
    archetype.reset(world->assign<T>());
    world->collect(this, previous);
    return *this;
}

template <typename T>
T *Entity::get() {
    auto it = components.find(get_type_id<T>());
    if (it != components.end()) {
        return std::any_cast<T>(&it->second);
    }
    return nullptr;
}

template <typename T>
bool Entity::has() {
    return components.find(get_type_id<T>()) != components.end();
}
