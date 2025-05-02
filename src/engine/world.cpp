#include "world.hpp"
#include "component.hpp"
#include "entity.hpp"
#include "system.hpp"
#include "logger.hpp"
#include <functional>
#include <cstring>

World::~World() {
    LOG_INFO("cleaning up ", pool.size(), " pooled entities and ", despawned.size(), " despawned entities");
    for (auto entity : pool)
        delete entity;
    for (auto entity : despawned)
        delete entity;
    for (auto& [_, archetype] : archetypes)
        delete archetype;
}

Entity* World::spawn() {
    Entity* entity = pool.empty() ? new Entity(++next_id, this) : pool.back();
    if (!pool.empty())
        pool.pop_back();
    
    ComponentMask empty_mask;
    Archetype* archetype = get_or_create_archetype(empty_mask);
    entity->archetype = archetype;
    entity->index = archetype->size++;
    if (archetype->size > archetype->capacity) {
        archetype->grow(archetype->capacity * 2);
    }
    archetype->entities.push_back(entity);
    
    LOG_DEBUG("Spawned entity ", entity->id, " (pooled: ", !pool.empty(), ")");
    return entity;
}

void World::despawn(Entity* entity) {
    LOG_DEBUG("Despawning entity ", entity->id);
    despawned.push_back(entity);
}

int World::assign(std::type_index type) {
    if (ids.find(type) == ids.end()) {
        ids[type] = next_bit++;
        LOG_DEBUG("Assigned new component type ", type.name(), " with ID ", ids[type]);
        return ids[type];
    }
    return ids[type];
}

Archetype* World::get_or_create_archetype(ComponentMask mask) {
    auto it = archetypes.find(mask);
    if (it != archetypes.end()) {
        return it->second;
    }
    
    Archetype* archetype = new Archetype(mask);
    archetypes[mask] = archetype;
    LOG_DEBUG("Created new archetype with mask ", mask.to_string());
    return archetype;
}

void World::move_to_archetype(Entity* entity, ComponentMask new_mask, void* component_data) {
    Archetype* old_archetype = entity->archetype;
    Archetype* new_archetype = get_or_create_archetype(new_mask);
    
    if (old_archetype == new_archetype) {
        return;
    }
    
    LOG_DEBUG("Moving entity ", entity->id, " from archetype ", old_archetype->mask.to_string(), 
              " to archetype ", new_archetype->mask.to_string());

    if (new_archetype->size >= new_archetype->capacity) {
        size_t new_capacity = new_archetype->capacity * 2;
        LOG_DEBUG("Growing archetype from ", new_archetype->capacity, " to ", new_capacity);
        new_archetype->grow(new_capacity);
    }
    
    // Copy existing components
    for (const auto& [type, array] : old_archetype->components) {
        if (new_mask.test(ids[type])) {
            // Ensure component array exists in new archetype
            if (new_archetype->components.find(type) == new_archetype->components.end()) {
                LOG_DEBUG("Creating component array of type ", type.name(), " with size ", old_archetype->component_sizes[type]);
                new_archetype->component_sizes[type] = old_archetype->component_sizes[type];
                new_archetype->components[type] = new char[new_archetype->capacity * old_archetype->component_sizes[type]];
            }
            
            void* src = static_cast<char*>(old_archetype->get_component_array(type)) + 
                       entity->index * old_archetype->component_sizes[type];
            void* dst = static_cast<char*>(new_archetype->get_component_array(type)) + 
                       new_archetype->size * new_archetype->component_sizes[type];
            std::memcpy(dst, src, old_archetype->component_sizes[type]);
        }
    }
    
    // Copy new component if provided
    if (component_data) {
        ComponentMask diff = new_mask & ~entity->mask;
        // Find which bit is set in the diff to identify the new component type
        for (const auto& [type, id] : ids) {
            if (diff.test(id)) {
                // Create component array if it doesn't exist
                if (new_archetype->components.find(type) == new_archetype->components.end()) {
                    new_archetype->component_sizes[type] = sizeof(void*);  // We'll get the actual size from the component array template
                    new_archetype->components[type] = new char[new_archetype->capacity * sizeof(void*)];
                }
                
                void* dst = static_cast<char*>(new_archetype->get_component_array(type)) + 
                           new_archetype->size * new_archetype->component_sizes[type];
                std::memcpy(dst, component_data, new_archetype->component_sizes[type]);
                LOG_DEBUG("Added new component of type ", type.name(), " to entity ", entity->id);
                break;
            }
        }
    }
    
    size_t old_index = entity->index;
    entity->index = new_archetype->size;
    entity->mask = new_mask;
    entity->archetype = new_archetype;
    new_archetype->entities.push_back(entity);
    new_archetype->size++;
    
    // Handle removal from old archetype
    if (old_archetype->size > 1 && old_index < old_archetype->size - 1) {
        old_archetype->move_entity(old_archetype->size - 1, old_index);
    }
    old_archetype->size--;
    old_archetype->entities.pop_back();
    
    // Clean up empty archetype if needed
    if (old_archetype->size == 0) {
        ComponentMask old_mask = old_archetype->mask;
        remove_archetype(old_mask);
    }
    
    sync(entity);
}

void World::add(System* system) {
    LOG_INFO("Adding system of type ", typeid(*system).name());
    system->compile(this);
    Entities* entities = new Entities();
    
    switch (system->mode) {
        case System::Mode::Ordered:
            ordered_systems.insert({system, entities});
            LOG_DEBUG("Added ordered system");
            break;
        case System::Mode::Unordered:
            unordered_systems.insert({system, entities});
            LOG_DEBUG("Added unordered system");
            break;
        case System::Mode::Render:
            render_systems.insert({system, entities});
            LOG_DEBUG("Added render system");
            break;
    }
    systems.insert({system, entities});
    
    // Add existing matching entities from all archetypes
    size_t matched_count = 0;
    for (auto& [_, archetype] : archetypes) {
        // If this archetype's mask matches the system's query
        if (system->query.matches(archetype->mask)) {
            // Add all entities from this archetype to the system
            for (Entity* entity : archetype->entities) {
                entities->insert(entity);
                matched_count++;
                LOG_DEBUG("Added existing entity ", entity->id, " to system ", typeid(*system).name());
            }
        }
    }
    LOG_DEBUG("System matched ", matched_count, " existing entities");
}

void World::remove(System* system) {
    LOG_INFO("Removing system of type ", typeid(*system).name());
    auto it = systems.find(system);
    if (it != systems.end()) {
        delete it->second;
    }
    
    ordered_systems.erase(system);
    unordered_systems.erase(system);
    render_systems.erase(system);
    systems.erase(system);
}

void World::sync(Entity* entity) {
    LOG_DEBUG("Syncing entity ", entity->id, " with all systems");
    for (auto& [system, entities] : systems) {
        sync(entity, system, entities);
    }
}

void World::sync(Entity* entity, System* system, Entities* entities) {
    bool matches = system->query.matches(entity->mask);
    bool contains = entities->find(entity) != entities->end();
    
    if (matches && !contains) {
        LOG_DEBUG("Entity ", entity->id, " added to system ", typeid(*system).name());
        entities->insert(entity);
    } else if (!matches && contains) {
        LOG_DEBUG("Entity ", entity->id, " removed from system ", typeid(*system).name());
        entities->erase(entity);
        system->cleanup(entity);
    }
}

void World::update(float delta) {
    if (!despawned.empty()) {
        LOG_DEBUG("Processing ", despawned.size(), " despawned entities");
    }
    
    std::unordered_map<ComponentMask, bool> empty_archetypes;
    
    for (Entity* entity : despawned) {
        if (entity->archetype) {
            ComponentMask mask = entity->archetype->mask;
            entity->archetype->remove_entity(entity->index);
            if (entity->archetype->size == 0) {
                empty_archetypes[mask] = true;
            }
        }
        entity->dispose();
        pool.push_back(entity);
    }
    despawned.clear();
    
    // Clean up empty archetypes after processing all despawned entities
    for (const auto& [mask, _] : empty_archetypes) {
        remove_archetype(mask);
    }
    
    LOG_DEBUG("Running sorted systems (delta: ", delta, ")");
    for (const auto& [system, entities] : sorted_systems) {
        for (int i = 0; i < system->subticks; i++) {
            system->execute(*entities, delta / system->subticks, this);
        }
    }
    
    LOG_DEBUG("Running unordered systems (delta: ", delta, ")");
    for (const auto& [system, entities] : unordered_systems) {
        for (int i = 0; i < system->subticks; i++) {
            system->execute(*entities, delta / system->subticks, this);
        }
    }
}

void World::render(float alpha) {
    LOG_DEBUG("Running render systems (alpha: ", alpha, ")");
    for (const auto& [system, entities] : render_systems) {
        system->execute(*entities, alpha, this);
    }
}

void World::compile() {

    for (auto &[system, entities] : ordered_systems) {
      for (auto &required : system->requires) {
        for (auto &[candidate, candidate_entities] : unordered_systems) {
          if (get_type_id(candidate) == required) {
            candidate->mode = System::Mode::Ordered;
            ordered_systems.insert({candidate, candidate_entities});
            unordered_systems.erase(candidate);
          }
        }
      }
    }
  
    sort_systems();
  }

void World::sort_systems() {
    LOG_DEBUG("Sorting ordered systems");
    std::unordered_map<System *, std::vector<System *>> graph;
  
    for (auto &[system, entities] : ordered_systems) {
      graph[system] = {};
    }
  
    for (auto &[system, entities] : ordered_systems) {
      for (auto &required : system->requires) {
        for (auto &[candidate, _] : ordered_systems) {
          if (get_type_id(candidate) == required) {
            graph[system].push_back(candidate);
          }
        }
      }
    }
  
    std::vector<std::pair<System *, Entities *>> sorted;
    std::unordered_map<System *, bool> visited;
    std::unordered_map<System *, bool> temp;
  
    std::function<void(System *)> visit = [&](System *system) {
      if (temp[system]) {
        throw std::runtime_error(
            "Circular dependency in ordered systems involving " +
            get_type_name(system));
      }
      if (!visited[system]) {
        temp[system] = true;
        for (auto &dep : graph[system]) {
          visit(dep);
        }
        visited[system] = true;
        temp[system] = false;
        sorted.push_back({system, ordered_systems[system]});
      }
    };
  
    for (auto &[system, _] : ordered_systems) {
      if (!visited[system]) {
        visit(system);
      }
    }
  
    sorted_systems = sorted;
  }

void World::remove_archetype(ComponentMask mask) {
    auto it = archetypes.find(mask);
    if (it != archetypes.end()) {
        LOG_DEBUG("Removing empty archetype with mask ", mask.to_string());
        delete it->second;
        archetypes.erase(it);
    }
}