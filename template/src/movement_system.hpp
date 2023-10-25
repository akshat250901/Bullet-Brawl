#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class MovementSystem
{
private:

public:
	void step(float elapsed_ms);

	MovementSystem()
	{
	}
};