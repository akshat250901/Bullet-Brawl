#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
private:
// ... (other private members and methods)

void checkCollisionBetweenPlayersAndPlatforms(float step_seconds);
void checkCollisionBetweenPlayersAndPowerups();
void checkCollisionBetweenPlayersAndBullets();

public:
	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}
};