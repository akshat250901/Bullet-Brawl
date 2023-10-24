// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Checks if the two rectangles intersect
bool collides(const Motion& motion1, const Motion& motion2)
{
    // Calculate the half width and half height for each rectangle
    float halfWidth1 = motion1.scale.x / 2.0f;
    float halfHeight1 = motion1.scale.y / 2.0f;
    float halfWidth2 = motion2.scale.x / 2.0f;
    float halfHeight2 = motion2.scale.y / 2.0f;

    // Check for overlap in the x-axis
    if (abs(motion1.position.x - motion2.position.x) < (halfWidth1 + halfWidth2)) {
        // Check for overlap in the y-axis
        if (abs(motion1.position.y - motion2.position.y) < (halfHeight1 + halfHeight2)) {
            return true; // The rectangles intersect
        }
    }
    return false; // The rectangles don't intersect
}

void PhysicsSystem::checkCollisionBetweenPlayersAndPlatforms(float step_seconds) {
    auto& motion_container = registry.motions;
    auto& players_container = registry.players;
    auto& platforms_container = registry.platforms;

    for(uint i = 0; i < players_container.components.size(); i++)
	{
		Entity entity_i = players_container.entities[i];
		Motion& motion_i = motion_container.get(entity_i);

		vec2 predicted_position_i = motion_i.position + (step_seconds * motion_i.velocity);
		
		// compare players to all platforms
		for(uint j = 0; j < platforms_container.components.size(); j++)
		{
			Entity entity_j = platforms_container.entities[j];
			Motion& motion_j = motion_container.get(entity_j);

			// Temporary update the player position to predicted position
        	vec2 original_position_i = motion_i.position;
        
        	motion_i.position = predicted_position_i;

			if (collides(motion_i, motion_j))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.playerPlatformCollisions.emplace_with_duplicates(entity_i, entity_j);
				registry.playerPlatformCollisions.emplace_with_duplicates(entity_j, entity_i);
			}

			// Restore the original positions
			motion_i.position = original_position_i;
		}
	}
}

void PhysicsSystem::checkCollisionBetweenPlayersAndPowerups() {
    auto& motion_container = registry.motions;
    auto& players_container = registry.players;
    auto& powerups_container = registry.powerUps;

    // Check for collisions between players and powerups
	for(uint i = 0; i < players_container.components.size(); i++)
	{
		Entity entity_i = players_container.entities[i];
		Motion& motion_i = motion_container.get(entity_i);
		
		// compare all players to all powerups
		for(uint j = 0; j < powerups_container.components.size(); j++)
		{
			Entity entity_j = powerups_container.entities[j];
			Motion& motion_j = motion_container.get(entity_j);

			if (collides(motion_i, motion_j))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.playerPowerUpCollisions.emplace_with_duplicates(entity_i, entity_j);
				registry.playerPowerUpCollisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}

void PhysicsSystem::checkCollisionBetweenPlayersAndBullets() {
    auto& motion_container = registry.motions;
    auto& players_container = registry.players;
    auto& bullets_container = registry.bullets;

    // Check for collisions between players and bullets
	for(uint i = 0; i < players_container.components.size(); i++)
	{
		Entity entity_i = players_container.entities[i];
		Motion& motion_i = motion_container.get(entity_i);
		
		// compare all players to all bullets
		for(uint j = 0; j < bullets_container.components.size(); j++)
		{
			Entity entity_j = bullets_container.entities[j];
			Motion& motion_j = motion_container.get(entity_j);

			if (collides(motion_i, motion_j))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.playerBulletCollisions.emplace_with_duplicates(entity_i, entity_j);
				registry.playerBulletCollisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}


void PhysicsSystem::step(float elapsed_ms)
{
	auto& motion_container = registry.motions;
	float step_seconds = elapsed_ms / 1000.f;

	//printf("elapsed_ms = %f\n", elapsed_ms);

	// Update positions of all objects based on velocities
	for(uint i = 0; i < motion_container.size(); i++)
	{
		Motion& motion = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];

		// Accelerate to change velocity if entity is player
		if(registry.players.has(entity_i)) {
			Player& player = registry.players.get(entity_i);
			if ((player.is_running_right) && (motion.velocity.x <= player.speed)) {
				motion.velocity.x += player.running_force * step_seconds;
			}
			if ((player.is_running_left) && (motion.velocity.x >= -player.speed)) {
				motion.velocity.x -= player.running_force * step_seconds;
			}

			// add a constant force for recoil, change later to add recoil force based on gun
			if (player.is_shooting && !player.facing_right) {
				motion.velocity.x += player.recoil_force * step_seconds;
				player.is_shooting = false;
			}
			if (player.is_shooting && player.facing_right) {
				motion.velocity.x -= player.recoil_force * step_seconds;
				player.is_shooting = false;
			}
		}

		motion.position += step_seconds * motion.velocity;
	}

	// Apply friction to all entities with friction component that just stopped moving
	auto& friction_container = registry.friction;

	for (uint i = 0; i < friction_container.size(); i++)
	{		
		// Get entity
		Entity entity_i = friction_container.entities[i];

		// Get player
		Player& player = registry.players.get(entity_i);

		// Get motion component
		Motion& motion = registry.motions.get(entity_i);

		const float deceleration_force = 5.0;
		const float opposite_direction_force = 3.0;

		if (motion.velocity.x != 0.0f) {
			
			// Only apply if player on ground
			if (player.is_grounded) {
				// Apply friction force to left if player just stops moving right
				if ((!player.is_running_right) && (motion.velocity.x > 0.0f)) {
					motion.velocity.x -= motion.velocity.x * deceleration_force * step_seconds;
				}

				// Apply friction force to right if player just stops moving left
				if ((!player.is_running_left) && (motion.velocity.x < 0.0f)) {
					motion.velocity.x -= motion.velocity.x * deceleration_force * step_seconds;
				}
			} 

			// Apply more force to slow down if opposite arrow keys are pressed
			if ((player.is_running_left) && (motion.velocity.x > 0.0f)) {
				motion.velocity.x -= motion.velocity.x * opposite_direction_force * step_seconds;
			}

			if ((player.is_running_right) && (motion.velocity.x < 0.0f)) {
				motion.velocity.x -= motion.velocity.x * opposite_direction_force * step_seconds;
			}
		}
	}


	auto& gravity_container = registry.gravity;

	// Apply gravity to all entities with gravity component
	for (uint i = 0; i < gravity_container.size(); i++)
	{
		Gravity& gravity = gravity_container.components[i];
		
		// Get player entity
		Entity entity_i = gravity_container.entities[i];

		// apply gravity force onto the motion component of the player
		Motion& motion = registry.motions.get(entity_i);
		motion.velocity += vec2(0.0f, gravity.force * step_seconds);
	}

	checkCollisionBetweenPlayersAndPlatforms(step_seconds);
    checkCollisionBetweenPlayersAndPowerups();
    checkCollisionBetweenPlayersAndBullets();
}