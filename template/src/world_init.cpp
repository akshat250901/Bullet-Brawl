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

Entity createBullet(bool isProjectile, vec2 pos, Entity& player)
{
	float bulletSpeed = 400.f; // Adjust the bullet's speed as needed
	float initialUpwardVelocity = 500.f; // Adjust the initial upward velocity as needed
	float angle = -45.f * M_PI/180;

	auto entity = Entity();
	Player& player_entity = registry.players.get(player);
	Motion& player_motion = registry.motions.get(player);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	if (player_entity.facing_right) {
		motion.position = {pos.x + (player_motion.scale.x / 2) + 25, pos.y};
	} else {
		motion.position = {pos.x - (player_motion.scale.x / 2) - 25, pos.y};
	}
	float vx = bulletSpeed * (player_entity.facing_right == 1 ? 1: -1);
	float vy = 0;
	if (isProjectile) {
		// Calculate the initial velocity components
		 vx = bulletSpeed * cos(angle) * (player_entity.facing_right == 1 ? 1 : -1);
		 vy = 2.f * bulletSpeed * sin(angle) + initialUpwardVelocity;
	}
	

	// Apply the initial upward velocity
	motion.velocity.x = vx;
	motion.velocity.y = vy;

	motion.scale = {15,5};

	registry.colors.insert(entity, { 255.0f, 255.0f, 255.0f });
	registry.bullets.emplace(entity);
	if (isProjectile)
	{
		registry.gravity.emplace(entity);
	}

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::SQUARE });

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

	registry.colors.insert(entity, color);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::SQUARE });

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
	parallax.scrollingSpeedFront = 0.0f;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FOREGROUND,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}