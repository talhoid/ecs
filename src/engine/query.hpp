#pragma once
#include "component.hpp"
#include <typeindex>
#include <vector>

class World;

class Query {
public:
    Query() = default;
    
    template<typename T>
    Query& has() {
        requires.push_back(get_type_id<T>());
        return *this;
    }
    
    template<typename T>
    Query& not_() {
        rejects.push_back(get_type_id<T>());
        return *this;
    }
    
    void compile(World* world);
    bool matches(ComponentMask mask) const;

private:
    std::vector<std::type_index> requires;
    std::vector<std::type_index> rejects;
    ComponentMask required;
    ComponentMask rejected;
};
