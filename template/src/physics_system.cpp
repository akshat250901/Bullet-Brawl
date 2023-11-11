// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <algorithm> // Include the algorithm header

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>

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

bool doLineSegmentIntersect(const glm::vec2& p1, const glm::vec2& q1, const glm::vec2& p2, const glm::vec2& q2) {
	glm::vec2 r = q1 - p1;
	glm::vec2 s = q2 - p2;

	float rxs = r.x * s.y - r.y * s.x;
	glm::vec2 qp = p2 - p1;

	float t = (qp.x * s.y - qp.y * s.x) / rxs;
	float u = (qp.x * r.y - qp.y * r.x) / rxs;

	if (rxs == 0 && glm::dot(qp, r) == 0) {
		// Lines are collinear
		float t0 = glm::dot(qp, r) / glm::dot(r, r);
		float t1 = t0 + glm::dot(s, r) / glm::dot(r, r);
		if ((t0 >= 0 && t0 <= 1) || (t1 >= 0 && t1 <= 1) || (t0 < 0 && t1 > 1) || (t0 > 1 && t1 < 0))
			return true;
	}

	return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

bool lineMeshCollision(const glm::vec2& lineStart, const glm::vec2& lineEnd, const std::vector<glm::vec2>& polygon, const Motion& motion) {
	// Transform the line using its start and end points
	glm::mat4 lineTransform = glm::mat4(1.0);
	lineTransform = glm::translate(lineTransform, glm::vec3(lineStart, 0.0));

	glm::vec2 transformedLineEnd = vec2(std::max(lineEnd.x, lineStart.x), std::min(lineEnd.y, lineStart.y));
	glm::vec2 transformedLineEnd_ = vec2(std::min(lineEnd.x, lineStart.x), std::max(lineEnd.y, lineStart.y));

	// Transform the polygon using its motion
	glm::mat4 polygonTransform = glm::mat4(1.0);
	polygonTransform = glm::translate(polygonTransform, glm::vec3(motion.position, 0.0));
	polygonTransform = glm::scale(polygonTransform, glm::vec3(motion.scale, 1.0));

	std::vector<glm::vec2> transformedPolygon;
	for (const auto& vertex : polygon) {
		transformedPolygon.push_back(glm::vec2(polygonTransform * glm::vec4(vertex, 0, 1)));
	}

	// Check for collision with each edge of the transformed polygon
	for (size_t i = 0; i < transformedPolygon.size(); ++i) {
		size_t nextIndex = (i + 1) % transformedPolygon.size();
		if (doLineSegmentIntersect(lineStart, transformedLineEnd, transformedPolygon[i], transformedPolygon[nextIndex]) || 
			doLineSegmentIntersect(lineStart, transformedLineEnd_, transformedPolygon[i], transformedPolygon[nextIndex])) {
			return true; // Intersection found
		}
	}

	return false; // No intersection with any edge
}

bool meshIntersectsMotion(const Mesh* mesh, Motion& bullet_motion, const Motion& object_motion) {
	// Iterate through each triangle defined by the vertex indices of the mesh
	for (size_t i = 0; i < mesh->vertex_indices.size(); i += 3) {
		// Get the vertices of the current triangle
		glm::vec2 vertex1 = glm::vec2(mesh->vertices[mesh->vertex_indices[i]].position.x, mesh->vertices[mesh->vertex_indices[i]].position.y);
		glm::vec2 vertex2 = glm::vec2(mesh->vertices[mesh->vertex_indices[i + 1]].position.x, mesh->vertices[mesh->vertex_indices[i + 1]].position.y);
		glm::vec2 vertex3 = glm::vec2(mesh->vertices[mesh->vertex_indices[i + 2]].position.x, mesh->vertices[mesh->vertex_indices[i + 2]].position.y);

		// Check if the transformed line segment intersects with the current triangle
		if (lineMeshCollision(object_motion.position - object_motion.scale / 2.0f, object_motion.position + object_motion.scale / 2.0f, { vertex1, vertex2, vertex3 }, bullet_motion)) {
			return true; // Collision detected with this triangle
		}
	}

	return false; // No collision detected with any triangle
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
			Bullet& bullet = registry.bullets.get(entity_j);

			if (!bullet.isHitscan) { // compute collisions only if bullet is not hitscan
				Motion& motion_j = motion_container.get(entity_j);
				const Mesh* bullet_mesh = registry.meshPtrs.get(entity_j);

				if (bullet.shooter != entity_i && meshIntersectsMotion(bullet_mesh, motion_j, motion_i))
				{
					// Create a collisions event
					// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
					registry.playerBulletCollisions.emplace_with_duplicates(entity_i, entity_j);
					registry.playerBulletCollisions.emplace_with_duplicates(entity_j, entity_i);
				}
			}
		}
	}
}


void PhysicsSystem::checkCollisionBetweenPlayersAndMysteryBoxes() {
    auto& motion_container = registry.motions;
    auto& players_container = registry.players;
    auto& mystery_box_container = registry.gunMysteryBoxes;

    // Check for collisions between players and mystery boxes
	for(uint i = 0; i < players_container.components.size(); i++)
	{
		Entity entity_i = players_container.entities[i];
		Motion& motion_i = motion_container.get(entity_i);
		
		// compare all players to all mystery boxes
		for(uint j = 0; j < mystery_box_container.components.size(); j++)
		{
			Entity entity_j = mystery_box_container.entities[j];
			Motion& motion_j = motion_container.get(entity_j);

			if (collides(motion_i, motion_j))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.playerMysteryBoxCollisions.emplace_with_duplicates(entity_i, entity_j);
				registry.playerMysteryBoxCollisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}



void PhysicsSystem::step(float elapsed_ms)
{
	auto& motion_container = registry.motions;
	float step_seconds = elapsed_ms / 1000.f;

	// Update positions of all objects based on velocities
	for(uint i = 0; i < motion_container.size(); i++)
	{
		Motion& motion = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];

		// // Accelerate to change velocity if entity is player // RECOIL WILL BE REFACTORED TO GUN SYSTEM
		// if(registry.players.has(entity_i)) {
		// 	Player& player = registry.players.get(entity_i);

		// 	// add a constant force for recoil, change later to add recoil force based on gun
		// 	if (player.is_shooting && !player.facing_right) {
		// 		motion.velocity.x += player.recoil_force * step_seconds;
		// 	}
		// 	if (player.is_shooting && player.facing_right) {
		// 		motion.velocity.x -= player.recoil_force * step_seconds;
		// 	}
		// }

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

		const float deceleration_force = 2.0;
		const float ground_friction = 3.0;
		const float opposite_direction_force = 3.0;

		if (motion.velocity.x != 0.0f) {

			int originalSign = (motion.velocity.x > 0.0f) ? 1 : (motion.velocity.x < 0.0f) ? -1 : 0; // Determine the original direction (+1 or -1)

			// Apply friction force to left if player just stops moving right
			if ((!player.is_running_right) && (motion.velocity.x > 0.0f)) {
				motion.velocity.x -= motion.velocity.x * deceleration_force * step_seconds;
			}

			// Apply friction force to right if player just stops moving left
			if ((!player.is_running_left) && (motion.velocity.x < 0.0f)) {
				motion.velocity.x -= motion.velocity.x * deceleration_force * step_seconds;
			}
			// Only apply if player on ground
			if (player.is_grounded) {
				// Apply friction force to left if player just stops moving right
				if ((!player.is_running_right) && (motion.velocity.x > 0.0f)) {
					motion.velocity.x -= motion.velocity.x * ground_friction * step_seconds;
				}

				// Apply friction force to right if player just stops moving left
				if ((!player.is_running_left) && (motion.velocity.x < 0.0f)) {
					motion.velocity.x -= motion.velocity.x * ground_friction * step_seconds;
				}
			}

			// Apply more force to slow down if opposite arrow keys are pressed
			if ((player.is_running_left) && (motion.velocity.x > 0.0f)) {
				motion.velocity.x -= motion.velocity.x * opposite_direction_force * step_seconds;
			}

			if ((player.is_running_right) && (motion.velocity.x < 0.0f)) {
				motion.velocity.x -= motion.velocity.x * opposite_direction_force * step_seconds;
			}

			// Ensure that the velocity doesn't change direction
			int newSign = (motion.velocity.x > 0.0f) ? 1 : (motion.velocity.x < 0.0f) ? -1 : 0;
			if (newSign != originalSign && newSign != 0) {
				motion.velocity.x = 0.0f; // Set velocity to zero if it changes direction
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
	checkCollisionBetweenPlayersAndMysteryBoxes();
}