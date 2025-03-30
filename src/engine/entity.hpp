#pragma once
#include "component.hpp"
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

class World;

class Entity {
    public:
    Entity(int id, World *world);
    
  template <typename T, typename... Args,
  typename std::enable_if_t<std::is_base_of_v<Component, T> &&
  std::is_constructible_v<T, Args...>> * =
                nullptr>
                Entity &add(Args &&...args);

  template <typename T, typename std::enable_if_t<
                            std::is_base_of_v<Component, T>> * = nullptr>
  Entity &remove();
  
  template <typename T, typename std::enable_if_t<
                            std::is_base_of_v<Component, T>> * = nullptr>
                            T *get();
                            
  template <typename T, typename std::enable_if_t<
                            std::is_base_of_v<Component, T>> * = nullptr>
                            bool has();
                            
                            void despawn();
  void dispose();
  
  Archetype archetype;

  const int id;
  private:
  World *world;
  std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
};