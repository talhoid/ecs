#include "archetypes.hpp"
#include "entity.hpp"
#include "logger.hpp"
#include <cstring>

Archetype::Archetype(ComponentMask mask) : mask(mask), size(0), capacity(16) {
    LOG_DEBUG("Created new archetype with initial capacity ", capacity);
    entities.reserve(capacity);
}

Archetype::~Archetype() {
    LOG_DEBUG("Destroying archetype with ", components.size(), " component arrays");
    for (auto& [type, array] : components) {
        delete[] static_cast<char*>(array);
    }
}

void* Archetype::get_component_array(std::type_index type) {
    auto it = components.find(type);
    if (it != components.end()) {
        return it->second;
    }
    LOG_WARNING("Attempted to get non-existent component array of type ", type.name());
    return nullptr;
}

size_t Archetype::get_size() const {
    return size;
}

void Archetype::grow(size_t new_size) {
    LOG_DEBUG("Growing archetype arrays from capacity ", capacity, " to ", new_size);
    for (auto& [type, old_array] : components) {
        size_t component_size = component_sizes[type];
        void* new_array = new char[new_size * component_size];
        std::memcpy(new_array, old_array, size * component_size);
        delete[] static_cast<char*>(old_array);
        components[type] = new_array;
    }
    entities.reserve(new_size);
    capacity = new_size;
}

void Archetype::remove_entity(size_t index) {
    if (index >= size) {
        LOG_ERROR("Attempted to remove entity at invalid index ", index, " (size: ", size, ")");
        return;
    }
    
    LOG_DEBUG("Removing entity at index ", index, " from archetype");
    
    // Move components
    for (auto& [type, array] : components) {
        size_t component_size = component_sizes[type];
        char* typed_array = static_cast<char*>(array);
        std::memmove(
            typed_array + index * component_size,
            typed_array + (size - 1) * component_size,
            component_size
        );
    }
    
    // Move entity pointer
    if (index < size - 1) {
        entities[index] = entities[size - 1];
        entities[index]->index = index;
    }
    entities.pop_back();
    size--;
}

void Archetype::move_entity(size_t from_index, size_t to_index) {
    if (from_index >= size || to_index >= size) {
        LOG_ERROR("Invalid entity move from index ", from_index, " to ", to_index, " (size: ", size, ")");
        return;
    }
    
    LOG_DEBUG("Moving entity from index ", from_index, " to ", to_index, " within archetype");
    
    // Move components
    for (auto& [type, array] : components) {
        size_t component_size = component_sizes[type];
        char* typed_array = static_cast<char*>(array);
        char* temp = new char[component_size];
        
        std::memcpy(temp, typed_array + from_index * component_size, component_size);
        std::memcpy(typed_array + to_index * component_size, temp, component_size);
        
        delete[] temp;
    }
    
    // Swap entity pointers and update their indices
    std::swap(entities[from_index], entities[to_index]);
    entities[from_index]->index = from_index;
    entities[to_index]->index = to_index;
}