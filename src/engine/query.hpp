#pragma once
#include "component.hpp"
#include "entity.hpp"
#include <unordered_set>

using Entities = std::unordered_set<Entity *>;

class Query {
public:
    template <typename T>
    Query &has() {
        requires.insert(get_type_id<T>());
        return *this;
    }

    template <typename T>
    Query &without() {
        rejects.insert(get_type_id<T>());
        return *this;
    }

    void compile(World *world);
    bool match(const ComponentMask *mask);
    Entities execute(std::unordered_map<ComponentMask, Entities> archetypes);

private:
    ComponentMask required = 0;
    ComponentMask rejected = 0;
    std::unordered_set<std::type_index> requires;
    std::unordered_set<std::type_index> rejects;
};
