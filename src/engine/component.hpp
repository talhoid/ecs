#pragma once
#include <bitset>
#include <typeindex>

constexpr size_t MAX_COMPONENTS = 8;
using ComponentMask = std::bitset<MAX_COMPONENTS>;

template <typename T> std::type_index get_type_id() {
  return std::type_index(typeid(T));
}

template <typename T> std::type_index get_type_id(const T &obj) {
  return std::type_index(typeid(obj));
}

template <typename T> std::type_index get_type_id(const T *obj) {
  if (!obj)
    throw std::bad_typeid();
  return std::type_index(typeid(*obj));
}

template <typename T> std::string get_type_name() {
  return get_type_id<T>().name();
}

template <typename T> std::string get_type_name(const T &obj) {
  return get_type_id(obj).name();
}
// Component types must be registered before use
// Example usage:
// struct Position {
//     float x, y;
//     static const int component_id;
// };
// const int Position::component_id = world.assign<Position>();