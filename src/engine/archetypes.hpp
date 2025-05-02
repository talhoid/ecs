#pragma once
#include "component.hpp"
#include <map>
#include <vector>
#include <typeindex>

class Entity;

class Archetype {
public:
    Archetype(ComponentMask mask);
    ~Archetype();

    template<typename T>
    T* get_component_array();

    template<typename T>
    void add_component_array();

    void* get_component_array(std::type_index type);
    size_t get_size() const;
    void grow(size_t new_size);
    void remove_entity(size_t index);
    void move_entity(size_t from_index, size_t to_index);
    
    ComponentMask mask;
    std::map<std::type_index, void*> components;
    std::map<std::type_index, size_t> component_sizes;
    std::vector<Entity*> entities;  // Store pointers to entities
    size_t size;
    size_t capacity;
};