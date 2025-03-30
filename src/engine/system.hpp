#pragma once
#include "query.hpp"
#include <vector>

class World;

class System {
public:
  enum Mode { Unordered, Ordered, Render };
  Mode mode = Mode::Unordered;

  std::vector<std::type_index>
    requires; // if the mode is ordered, for ordered systems
  Query query;

  virtual ~System() = default;
  virtual void compile(World *world) = 0;
  virtual void execute(const Entities &entities, float delta, World *world) = 0;
  virtual void cleanup(Entity *entity) {}
};