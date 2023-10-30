#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	motion.scale = vec2({ 60, 60 });

	// Add gravity component
	Gravity& gravity = registry.gravity.emplace(entity);

	// Add horizontal friction component
	Friction& friction = registry.friction.emplace(entity);

	// Add animated sprite component
	AnimatedSprite& animated_sprite = registry.animatedSprite.emplace(entity);
	animated_sprite.frame_count_per_type = { {0, 8}, {1, 8}, {2, 2}, {3, 1} };

	registry.players.emplace(entity);

	registry.playerStatModifiers.emplace(entity);

	registry.controllers.emplace(entity);

	for (int i = 0; i < 5; i++) {
		float start_x = 50.f;
		TEXTURE_ASSET_ID health = TEXTURE_ASSET_ID::RED_HEALTH;
		if (registry.players.size() > 1) {
			start_x = 900;
			health = TEXTURE_ASSET_ID::GREEN_HEALTH;
		}
		auto health_entity = Entity();
		registry.lives.emplace(health_entity, entity);
		auto& health_motion = registry.motions.emplace(health_entity);
		health_motion.angle = 0;
		health_motion.velocity = { 0.f, 0.f };
		health_motion.position = { start_x + i * 60.f, 50.f };
		health_motion.scale = { 50.f, 50.f };
		registry.renderRequests.insert(
			health_entity,
			{ health,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_SPRITESHEET,
		 EFFECT_ASSET_ID::PLAYER,
		 GEOMETRY_BUFFER_ID::ANIMATED_SPRITE });

	return entity;
}


Entity createPlatform(RenderSystem* renderer, vec3 color, vec2 position, vec2 size)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = size;
	registry.platforms.emplace(entity);
	//registry.colors.insert(entity, color);
	// registry.renderRequests.insert(
	// 	entity,
	// 	{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
	// 		EFFECT_ASSET_ID::COLOURED,
	// 		GEOMETRY_BUFFER_ID::SQUARE });

	return entity;
}

// Function to calculate vx and vy for projectiles
std::tuple<float, float> calculateProjectileVelocity(float bulletSpeed, float angle, int facingRight, vec2 playerVelocity, float initialUpwardVelocity)
{
	float vx = (bulletSpeed * cos(angle) + abs(playerVelocity.x) * 2.f) * (facingRight == 1 ? 1 : -1);
	float vy = 2.f * bulletSpeed * sin(angle) + initialUpwardVelocity + abs(playerVelocity.x);
	return std::make_tuple(vx, vy);
}

Entity createBullet(RenderSystem* renderer, bool isProjectile, vec2 pos, Entity& player)
{
	float bulletSpeed = 750.f;
	float initialUpwardVelocity = 50.f;
	float angle = -60.f * M_PI / 180;

	auto entity = Entity();
	Player& player_entity = registry.players.get(player);
	Motion& player_motion = registry.motions.get(player);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BULLET);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);

	// Calculate vx and vy based on whether it's a projectile
	float vx, vy;
	if (isProjectile) {
		bulletSpeed = 400.f;
		std::tie(vx, vy) = calculateProjectileVelocity(bulletSpeed, angle, player_entity.facing_right, player_motion.velocity, initialUpwardVelocity);
	}
	else {
		vx = bulletSpeed * (player_entity.facing_right == 1 ? 1 : -1);
		vy = 0;
	}

	motion.velocity.x = vx;
	motion.velocity.y = vy;

	motion.scale = { 20, 10 };
	motion.position = { player_entity.facing_right == 1 ? pos.x + player_motion.scale.x / 2 : pos.x - player_motion.scale.x / 2 , pos.y };

	registry.colors.insert(entity, { 255.0f, 255.0f, 255.0f });
	registry.bullets.emplace(entity, player);

	if (isProjectile)
	{
		motion.scale = { 30, 20 };
		motion.position = { pos.x , pos.y - player_motion.scale.y / 2 - 20 };
		registry.gravity.emplace(entity);
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
				EFFECT_ASSET_ID::COLOURED,
				GEOMETRY_BUFFER_ID::PROJECTILE });
		return entity;
	}

	if (player_entity.facing_right) {
		motion.angle = -180 * M_PI / 180;
	}

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::BULLET });

	return entity;
}

Entity createPowerup(RenderSystem* renderSystem, vec2 pos, vec2 scale, vec3 color)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderSystem->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	motion.scale = scale;

	Interpolation& interpolation = registry.interpolation.emplace(entity);
	interpolation.startPosition = pos;
	interpolation.endPosition = { pos.x + 100, pos.y };
	interpolation.currentTime = 0;

	// Add animated sprite component
	AnimatedSprite& animated_sprite = registry.animatedSprite.emplace(entity);
	animated_sprite.frame_count_per_type = { {0, 6}, {1, 6}, {2, 6}, {3, 6} };
	animated_sprite.animation_type = 0;
	animated_sprite.animation_speed_ms = 150;

	registry.colors.insert(entity, color);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::POWERUP_SPRITESHEET,
			EFFECT_ASSET_ID::ANIMATED,
			GEOMETRY_BUFFER_ID::ANIMATED_SPRITE });


	return entity;
}

Entity createBackgroundIsland(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 position, vec2 size)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = size;

	// Add the Parallax component for the back layer, which might move the slowest.
	ParallaxBackground& parallax = registry.parallaxes.emplace(entity);
	if (game_state_system->get_current_state() == 1) {
		registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLATFORM,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });
	} else if (game_state_system->get_current_state() == 2) {
		registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TUTORIALPLATFORM,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}

Entity createBackgroundBack(RenderSystem* renderer, vec2 position, vec2 size)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = size;

	// Add the Parallax component for the back layer, which might move the slowest.
	ParallaxBackground& parallax = registry.parallaxes.emplace(entity);
	parallax.scrollingSpeedBack = 0.05f; // Adjust this value as needed.
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BACKGROUND,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBackgroundMiddle(RenderSystem* renderer, vec2 position, vec2 size)
{
	// Reserve an entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = size;

	// Add the Parallax component for the middle layer.
	ParallaxBackground& parallax = registry.parallaxes.emplace(entity);
	parallax.scrollingSpeedMiddle = 0.08f; // Adjust this value as needed.

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MIDDLEGROUND,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBackgroundForeground(RenderSystem* renderer, vec2 position, vec2 size)
{

	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = size;

	// Add the Parallax component for the back layer, which might move the slowest.
	ParallaxBackground& parallax = registry.parallaxes.emplace(entity);
	parallax.scrollingSpeedFront = 0.3f;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FOREGROUND,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}