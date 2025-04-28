#pragma once
#include "component.hpp"
#include <unordered_map>
#include <typeindex>
#include <any>

class World;

class Entity {
public:
    Entity(int id, World *world);

    template <typename T, typename... Args,
              typename std::enable_if_t<std::is_constructible_v<T, Args...>> * = nullptr>
    Entity &add(Args&&... args);

    template <typename T>
    Entity &remove();

    template <typename T>
    T *get();

    template <typename T>
    bool has();

    void despawn();
    void dispose();

    ComponentMask archetype;
    const int id;

private:
    World *world;
    std::unordered_map<std::type_index, std::any> components;
};