// internal
#include "movement_system.hpp"
#include "world_init.hpp"

const float JUMP_DELAY_MS = 300.0f;

void MovementSystem::step(float elapsed_ms)
{
    // Handle movement for all entities that are controllable

    auto& controllable_container = registry.controllers;

    for (int i = 0; i < controllable_container.size(); i++) {
        Controller& controller_i = controllable_container.components[i];
        Entity entity_i = controllable_container.entities[i];

        Player& player_i = registry.players.get(entity_i);
        Motion& motion_i = registry.motions.get(entity_i);

        bool rightKey = controller_i.rightKey;
        bool leftKey = controller_i.leftKey;

        bool downKey = controller_i.downKey;
        bool upKey = controller_i.upKey;

        //Handle inputs for left and right arrow keys
		if (rightKey && !leftKey) {
			player_i.is_running_right = true;
			player_i.is_running_left = false;
			player_i.facing_right = true;
		}
		else if (!rightKey && leftKey) {
			player_i.is_running_left = true;
			player_i.is_running_right = false;
			player_i.facing_right = false;
		}
		else if ((!rightKey && !leftKey) || (rightKey && leftKey)) {
			player_i.is_running_left = false;
			player_i.is_running_right = false;
		}

        controller_i.upKey_delay_timer_ms -= elapsed_ms;

		// Handle up arrow input for jumping
		if (upKey) {
			if (player_i.is_grounded) {
				motion_i.velocity.y = -player_i.jump_force;
                controller_i.upKey_delay_timer_ms = JUMP_DELAY_MS;
			}
			else if (player_i.jump_remaining > 0 && controller_i.upKey_delay_timer_ms <= 0) {
				motion_i.velocity.y = -player_i.jump_force;
				player_i.jump_remaining--;
                controller_i.upKey_delay_timer_ms = JUMP_DELAY_MS;
			}
		}

		if (downKey) {
			if (player_i.is_grounded) {
				motion_i.position.y += 1.0f;
			}
		}

        float step_seconds = elapsed_ms / 1000.f;

        if (rightKey && (motion_i.velocity.x <= player_i.speed)) {
				motion_i.velocity.x += player_i.running_force * step_seconds;
		}

        if (leftKey && (motion_i.velocity.x >= -player_i.speed)) {
            motion_i.velocity.x -= player_i.running_force * step_seconds;
        }


    }
}