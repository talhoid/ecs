#pragma once
#include "component.hpp"

class World;
class Archetype;

class Entity {
public:
    Entity(int id, World* world);

    template<typename T, typename... Args>
    Entity& add(Args&&... args);

    template<typename T>
    Entity& remove();

    template<typename T>
    T* get();

    template<typename T>
    bool has();

    void despawn();
    void dispose();

    const int id;
    ComponentMask mask;
    Archetype* archetype;
    size_t index;

private:
    World* world;
};