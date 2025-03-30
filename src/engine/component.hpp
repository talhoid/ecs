#pragma once
#include <bitset>
#include <typeindex>


class Component {
public:
  virtual ~Component() = default;
};

template <typename T>
 std::type_index get_type_id() {
  return std::type_index(typeid(T));
}

using Archetype = std::bitset<256>;