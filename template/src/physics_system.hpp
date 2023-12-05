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
void checkCollisionBetweenPlayersAndMysteryBoxes();
bool predictCollisionBetweenPlayerAndBullet(const Mesh* mesh, Motion& motion_i, Motion& motion_j, float timeToCollision);

public:
	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}
};