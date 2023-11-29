#pragma once

#include "common.hpp"

#include "tiny_ecs.hpp"

#include "components.hpp"

#include "tiny_ecs_registry.hpp"


class RocketSystem

{

public:

    void step(float elapsed_ms);

    void updateBezierMotion(Entity& entity, float deltaTime);

    void updatePathfinding(Entity& entity, float deltaTime);

    vec2 interpolate(const std::vector<vec2>& points, float t);

};