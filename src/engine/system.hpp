#pragma once
#include "query.hpp"
#include "entities.hpp"
#include <vector>

class World;

class System {
public:
  enum Mode { Unordered, Ordered, Render };
  Mode mode = Mode::Unordered;
  // probably should have like
  // enum RenderMode { None, Ordered, Unordered };
  // RenderMode render_mode = RenderMode::None
  // instead of arbitrary ordering of all render systems

  std::vector<std::type_index>
    requires; // if the mode is ordered, for ordered systems
  Query query;

  int subticks = 1;

  virtual ~System() = default;
  virtual void compile(World *world) = 0;
  virtual void execute(const Entities &entities, float delta, World *world) = 0;
  virtual void cleanup(Entity *entity) {};
};