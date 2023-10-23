// internal
#include "animation_system.hpp"
#include "world_init.hpp"

void setAnimationFrame(int type, Player& player, AnimatedSprite& as) {
	player.movement_state = type;
	as.animation_type = type;
}

void AnimationSystem::step(float elapsed_ms_since_last_update)
{
	auto& player_container = registry.players;

	for (uint i = 0; i < player_container.size(); i++)
	{
		Entity entity_i = player_container.entities[i];
		Player& player = registry.players.get(entity_i);
		Motion& motion = registry.motions.get(entity_i);
		if (registry.animatedSprite.has(entity_i)) {
			AnimatedSprite& animated_sprite = registry.animatedSprite.get(entity_i);
			if (!player.is_grounded && (motion.velocity.y <= 0.f)) // jumping up
			{
				setAnimationFrame(2, player, animated_sprite);
			}
			else if (!player.is_grounded && (motion.velocity.y > 0.f)) // falling
			{
				setAnimationFrame(3, player, animated_sprite);
			}
			else if (player.is_running_left || player.is_running_right)
			{
				setAnimationFrame(1, player, animated_sprite);
			}
			else
			{
				setAnimationFrame(0, player, animated_sprite);
			}
			manageSpriteFrame(elapsed_ms_since_last_update, entity_i);
		}
	}
}

void AnimationSystem::manageSpriteFrame(float elapsed_ms_since_last_update, Entity entity) {
	if (registry.animatedSprite.has(entity)) {
		AnimatedSprite& animated_sprite = registry.animatedSprite.get(entity);
		animated_sprite.ms_since_last_update += elapsed_ms_since_last_update;

		if (animated_sprite.ms_since_last_update > animated_sprite.animation_speed_ms) {
			int frame_count = animated_sprite.frame_count_per_type[animated_sprite.animation_type];
			animated_sprite.animation_frame = (animated_sprite.animation_frame + 1) % frame_count; // mod is to loop back the frame num
			animated_sprite.ms_since_last_update = 0;
		}
	}
}


