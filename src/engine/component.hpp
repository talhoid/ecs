#pragma once
#include <bitset>
#include <typeindex>

class Component {
public:
  virtual ~Component() = default;
};

template <typename T> std::type_index get_type_id() {
  return std::type_index(typeid(T));
}

template <typename T> std::type_index get_type_id(const T &obj) {
  return std::type_index(typeid(*obj));
}

template <typename T> std::string get_type_name() {
  return get_type_id<T>().name();
}
template <typename T> std::string get_type_name(const T &obj) {
  return get_type_id(obj).name();
}

using Archetype = std::bitset<256>;