// internal
#include "animation_system.hpp"
#include "world_init.hpp"


void AnimationSystem::step()
{
	auto& player_container = registry.players;

	for (uint i = 0; i < player_container.size(); i++)
	{
		Entity entity_i = player_container.entities[i];
		Player& player = registry.players.get(entity_i);

		// jumping
		if (!player.is_grounded)
		{
			player.movement_state = 2;
		}
		else if (player.is_running_left || player.is_running_right)
		{
			player.movement_state = 1;
		}
		else
		{
			player.movement_state = 0;
		}
	}
}