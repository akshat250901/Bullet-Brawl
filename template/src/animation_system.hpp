#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class AnimationSystem
{
public:
	void step(float elapsed_ms_since_last_update);
	void manageSpriteFrame(float elapsed_ms_since_last_update, Entity entity);

	AnimationSystem()
	{
	}
};