#pragma once
#include "component.hpp"
#include "entities.hpp"
#include "system.hpp"
#include "archetypes.hpp"
#include <unordered_map>
#include <vector>

class Entity;

class World {
public:
    World() : next_id(0), next_bit(0) {}
    ~World();

    Entity* spawn();
    void despawn(Entity* entity);

    template<typename T>
    int assign() { return assign(get_type_id<T>()); }

    int assign(std::type_index type);
    void move_to_archetype(Entity* entity, ComponentMask new_mask, void* component_data);
    Archetype* get_or_create_archetype(ComponentMask mask);

    void add(System* system);
    void remove(System* system);
    void update(float delta);
    void render(float alpha);
    void compile();

private:
    int next_id;
    int next_bit;
    std::unordered_map<std::type_index, int> ids;
    std::unordered_map<ComponentMask, Archetype*> archetypes;
    std::unordered_map<System*, Entities*> systems;
    std::unordered_map<System*, Entities*> ordered_systems;
    std::unordered_map<System*, Entities*> unordered_systems;
    std::unordered_map<System*, Entities*> render_systems;
    std::vector<std::pair<System*, Entities*>> sorted_systems;
    std::vector<Entity*> pool;
    std::vector<Entity*> despawned;

    void sync(Entity* entity);
    void sync(Entity* entity, System* system, Entities* entities);
    void remove_archetype(ComponentMask mask);
    void sort_systems();
};