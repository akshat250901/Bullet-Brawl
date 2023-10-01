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

void PhysicsSystem::step(float elapsed_ms)
{
	auto& motion_container = registry.motions;
	float step_seconds = elapsed_ms / 1000.f;

	// Update positions of all objects based on velocities
	for(uint i = 0; i < motion_container.size(); i++)
	{
		Motion& motion = motion_container.components[i];
		motion.position += step_seconds * motion.velocity;
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

		/*Player& player = registry.players.get(entity_i);*/
	}

	// Predict the collisions between all moving entities
	for(uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];

		vec2 predicted_position_i = motion_i.position + (step_seconds * motion_i.velocity);
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];

			vec2 predicted_position_j = motion_j.position + motion_j.velocity;

			// Temporary update the positions to their predicted positions, we have to check for collisions the moment before they happen
			// to determine how to handle the collision
        	vec2 original_position_i = motion_i.position;
        	vec2 original_position_j = motion_j.position;
        
        	motion_i.position = predicted_position_i;
        	motion_j.position = predicted_position_j;

			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}

			// Restore the original positions
			motion_i.position = original_position_i;
        	motion_j.position = original_position_j;
		}
	}
}