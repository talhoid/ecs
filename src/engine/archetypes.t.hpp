#pragma once
#include "archetypes.hpp"
#include "logger.hpp"

template<typename T>
T* Archetype::get_component_array() {
    auto type = get_type_id<T>();
    if (components.find(type) == components.end()) {
        LOG_DEBUG("Creating component array of type ", type.name(), " with size ", sizeof(T));
        component_sizes[type] = sizeof(T);
        components[type] = new char[capacity * sizeof(T)];
    }
    return static_cast<T*>(components[type]);
}

template<typename T>
void Archetype::add_component_array() {
    auto type = get_type_id<T>();
    if (components.find(type) != components.end()) {
        return;
    }
    
    LOG_DEBUG("Adding component array of type ", type.name(), " with size ", sizeof(T));
    component_sizes[type] = sizeof(T);
    components[type] = new char[capacity * sizeof(T)];
}