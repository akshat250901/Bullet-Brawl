#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <string>

Entity createPlayer(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 pos)
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
	registry.invincibility.emplace(entity);

	registry.controllers.emplace(entity);
	
	if (game_state_system->get_current_state() == 2) {
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
			/*registry.renderRequests.insert(
				health_entity,
				{ health,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });*/
		}
	}


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_SPRITESHEET,
		 EFFECT_ASSET_ID::PLAYER,
		 GEOMETRY_BUFFER_ID::ANIMATED_SPRITE });

	return entity;
}

Entity createOutOfBoundsArrow(RenderSystem* renderer, Entity player, bool isPlayer1) 
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	OutOfBoundsArrow& arrow = registry.outOfBoundsArrows.emplace(entity);
	arrow.entity_to_track = player;
	arrow.textureId = (isPlayer1 ? TEXTURE_ASSET_ID::GREEN_ARROW : TEXTURE_ASSET_ID::RED_ARROW);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = { 0.f, 0.f };

	motion.scale = { 50.0f, 50.0f };

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

	return entity;
}

Entity createPopupIndicator(RenderSystem* renderer, std::string popup_type, Entity& player)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial values, scale is negative to make it face the opposite way
	registry.popupIndicator.emplace(entity);
	PopupIndicator& popup = registry.popupIndicator.get(entity);
	Motion& popup_motion = registry.motions.emplace(entity);

	popup_motion.scale = { 150, 50 };
	popup.player = player;

	TEXTURE_ASSET_ID texture_id;

	popup.type = popup_type;

	if (popup_type == "SUBMACHINE GUN") {
		texture_id = TEXTURE_ASSET_ID::SMG_PICKUP;
	}
	else if (popup_type == "ASSAULT RIFLE") {
		texture_id = TEXTURE_ASSET_ID::AR_PICKUP;
	}
	else if (popup_type == "SNIPER RIFLE") {
		texture_id = TEXTURE_ASSET_ID::SNIPER_PICKUP;
	}
	else if (popup_type == "SHOTGUN") {
		texture_id = TEXTURE_ASSET_ID::SHOTGUN_PICKUP;
	}
	else if (popup_type == "Triple Jump") {
		texture_id = TEXTURE_ASSET_ID::TRIPLEJUMP;
	}
	else if (popup_type == "Speed Boost") {
		texture_id = TEXTURE_ASSET_ID::SPEEDBOOST;
	}
	else if (popup_type == "Super Jump") {
		texture_id = TEXTURE_ASSET_ID::SUPERJUMP;
	}
	else if (popup_type == "Reload") {
		texture_id = TEXTURE_ASSET_ID::RELOAD_TEXT;
	}
	else {
		// not a valid popup_type
		// should technically throw error but in the case of the game 
		// its fine to just not render anything

		return entity;
	}

	registry.renderRequests.insert(
		entity,
		{ texture_id, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

// Function to calculate vx and vy for projectiles
std::tuple<float, float> calculateProjectileVelocity(float bulletSpeed, float angle, int facingRight, vec2 playerVelocity, float initialUpwardVelocity)
{
	float vx = (bulletSpeed * cos(angle) + abs(playerVelocity.x) * 2.f) * (facingRight == 1 ? 1 : -1);
	float vy = 2.f * bulletSpeed * sin(angle) + initialUpwardVelocity + abs(playerVelocity.x);
	return std::make_tuple(vx, vy);
}

// COMMENT OUT THIS AND CREATE NEW CREATE BULLET AND CREATE PROJECTILE IF NEEDED
// CREATE BULLET SHOULD TAKE GUN COMPONENT

Entity createBullet(RenderSystem* renderer, Entity gunEntity) {
	Motion& gunMotion = registry.motions.get(gunEntity);
	Gun& gunComponent = registry.guns.get(gunEntity);
	Entity gunOwner = gunComponent.gunOwner;
	Player& player = registry.players.get(gunOwner);

	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BULLET);
 	registry.meshPtrs.emplace(entity, &mesh);

 	Motion& bulletMotion = registry.motions.emplace(entity);

	bulletMotion.velocity.x = gunComponent.bulletVelocity * (player.facing_right == 1 ? 1 : -1);
	bulletMotion.scale = { 20, 10 }; // CAN ADD BULLET SIZE MODIFIER INTO GUN LATER
	if (player.facing_right) {
		bulletMotion.angle = -180 * M_PI / 180;
	}

	// Spawn bullet at the start of the gun barrel
	vec2 posGun = gunMotion.position;
	vec2 gunSize = gunMotion.scale;
	float bulletSpawnXPos = player.facing_right == 1 ? posGun.x + gunSize.x / 2 : posGun.x - gunSize.x / 2;
 	bulletMotion.position = { bulletSpawnXPos, posGun.y };

	// Set color of bullet
	registry.colors.insert(entity, { 255.0f, 255.0f, 255.0f });

	// Set bullet stats
	Bullet& bullet = registry.bullets.emplace(entity);
 	bullet.shooter = gunOwner;
	bullet.originalXPosition = bulletSpawnXPos;
	bullet.hasNormalDropOff = gunComponent.hasNormalDropOff;
	bullet.distanceStrengthModifier = gunComponent.distanceStrengthModifier;
	bullet.knockback = gunComponent.knockback;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::BULLET });

	return entity;
}


Entity createProjectile(RenderSystem* renderer, bool isProjectile, vec2 pos, Entity& player)
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
	
	Bullet& bullet = registry.bullets.emplace(entity);
	bullet.shooter = player;

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

Entity createGunMysteryBox(RenderSystem* renderSystem, vec2 pos, vec2 scale)
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

	/*registry.colors.insert(entity, {0.0f, 255.0f, 0.0f});*/

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WEAPON_BOX,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createGun(RenderSystem* renderSystem, vec2 scale, std::string gun_name)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderSystem->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.scale = scale;
	motion.velocity = { 0.f, 0.f };

	if (gun_name == "SUBMACHINE GUN") {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::SMG,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (gun_name == "ASSAULT RIFLE") {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::AR,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (gun_name == "SNIPER RIFLE") {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::SNIPER,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (gun_name == "SHOTGUN") {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::SHOTGUN,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		// default case should be pistol
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::PISTOL,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}



	return entity;
}

Entity createMuzzleFlash(RenderSystem* renderSystem, Motion& motion, bool facing_right)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderSystem->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion_return = registry.motions.insert(entity, motion);
	motion_return.velocity = { 0.f, 0.f };

	registry.muzzleFlashes.emplace(entity);

	registry.muzzleFlashes.get(entity).facing_right = facing_right;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SHOTGUN_MUZZLE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

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
	if (game_state_system->get_current_state() == 2) {
		registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLATFORM,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });
	} else if (game_state_system->get_current_state() == 3) {
		registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TUTORIALMAP,
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

Entity createBackgroundJungle(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 position, vec2 size)
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
	if (game_state_system->get_current_state() == 2) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::JUNGLEMAP,
				EFFECT_ASSET_ID::BACKGROUND,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (game_state_system->get_current_state() == 3) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TUTORIALMAP,
				EFFECT_ASSET_ID::BACKGROUND,
				GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}

Entity createBackgroundSpace(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 position, vec2 size)
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
	if (game_state_system->get_current_state() == 2) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::SPACEMAP,
				EFFECT_ASSET_ID::BACKGROUND,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (game_state_system->get_current_state() == 3) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TUTORIALMAP,
				EFFECT_ASSET_ID::BACKGROUND,
				GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}

Entity createBackgroundTemple(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 position, vec2 size)
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
	if (game_state_system->get_current_state() == 2) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TEMPLEMAP,
				EFFECT_ASSET_ID::BACKGROUND,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (game_state_system->get_current_state() == 3) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TUTORIALMAP,
				EFFECT_ASSET_ID::BACKGROUND,
				GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}

Entity createBackgroundTutorial(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 position, vec2 size)
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

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TUTORIALMAP,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });
	
	return entity;
}

Entity createText(std::string text, vec2 position, vec3 color, float scale, float opacity, int horizontalAlignment, int verticalAlignment, Entity owner, std::string tag, float timer) {

	// Reserve en entity
	auto entity = Entity();

	Text& textObj = registry.texts.emplace(entity);
	textObj.string = text;
	textObj.position = position;
	textObj.color = color;
	textObj.scale = scale;
	textObj.opacity = opacity;
	textObj.horizontalAlignment = horizontalAlignment;
	textObj.verticalAlignment = verticalAlignment;
	textObj.owner = owner;
	textObj.tag = tag;
	textObj.timer_ms = timer;
	// Put into motion but do nothing
	registry.motions.emplace(entity);
	if (timer != -1) {
		// Text for players death log
		registry.deathLog.emplace(entity);
	}

	return entity;
}

void createTutorialMap(RenderSystem* renderer, GameStateSystem* game_state_system, int window_width_px, int window_height_px)
{
	createBackgroundTutorial(renderer, game_state_system, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 600, 314 }, { 792, 10 }); // Top
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 260, 467 }, { 230, 10 }); // Middle left
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 940, 467 }, { 230, 10 }); // Middle right 
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 600, 633 }, { 792, 10 }); // Bottom
}

void createIslandMap(RenderSystem* renderer, GameStateSystem* game_state_system, int window_width_px, int window_height_px)
{
	createBackgroundBack(renderer, { window_width_px / 2, window_height_px / 2 }, { window_width_px + 200, window_height_px });
	createBackgroundMiddle(renderer, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });
	createBackgroundForeground(renderer, { window_width_px / 2,window_height_px / 2 }, { window_width_px, window_height_px });
	createBackgroundIsland(renderer, game_state_system, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 390, 130 }, { 320, 10 }); // Top
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 415, 220 }, { 470, 10 }); // Second
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 470, 310 }, { 616, 10 }); // Third 
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 530, 415 }, { 800, 10 }); // Fourth
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 590, 530 }, { 1011, 10 }); // Bottom
}

void createJungleMap(RenderSystem* renderer, GameStateSystem* game_state_system, int window_width_px, int window_height_px)
{
	createBackgroundJungle(renderer, game_state_system, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 240, 190 }, { 300, 10 }); // Top left
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 830, 210 }, { 530, 10 }); // Top right
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 550, 305 }, { 960, 10 }); // long boi
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 1005, 420 }, { 340, 10 }); // middle right
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 330, 420 }, { 300, 10 }); // middle left
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 779, 525 }, { 598, 10 }); // below middle right
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 426, 630 }, { 596, 10 }); // bottom left
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 970, 630 }, { 250, 10 }); // bottom right
}

void createSpaceMap(RenderSystem* renderer, GameStateSystem* game_state_system, int window_width_px, int window_height_px)
{
	createBackgroundSpace(renderer, game_state_system, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 365, 265 }, { 300, 10 }); // Top left
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 940, 285 }, { 270, 10 }); // Top right
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 635, 418 }, { 740, 10 }); // middle
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 248, 534 }, { 280, 10 }); // level 3 left
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 910, 534 }, { 260, 10 }); // level 3 right
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 396, 642 }, { 255, 10 }); // level 4 left
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 810, 650 }, { 230, 10 }); // level 4 right
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 600, 740 }, { 230, 10 }); // bottom
}

void createTempleMap(RenderSystem* renderer, GameStateSystem* game_state_system, int window_width_px, int window_height_px)
{
	createBackgroundTemple(renderer, game_state_system, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 720, 305 }, { 360, 1 }); // Top
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 600, 400 }, { 900, 1 }); // long
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 260, 505 }, { 380, 10 }); // long
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 850, 520 }, { 380, 10 }); // long
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 530, 620 }, { 800, 10 }); // long
}

void createDeathScreen(RenderSystem* renderer, GameStateSystem* game_state_system, const vec2& position, const vec2& size) {
	// Reserve an entity
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
	parallax.scrollingSpeedBack = 0.0f; // Adjust this value as needed.
	if (game_state_system->get_winner() == 1) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::RED_PLAYER_WON,
			  EFFECT_ASSET_ID::BACKGROUND,
			  GEOMETRY_BUFFER_ID::SPRITE });
	} else if (game_state_system->get_winner() == 2) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::GREEN_PLAYER_WON,
			  EFFECT_ASSET_ID::BACKGROUND,
			  GEOMETRY_BUFFER_ID::SPRITE });
	}
	auto player1 = createPlayer(renderer, game_state_system, { 900, 300 });
	registry.players.get(player1).color = { 1.f, 0.f, 0.f };
	auto player2 = createPlayer(renderer, game_state_system, { 300, 200 });
	registry.players.get(player2).color = { 0.f, 1.f, 0.f };
}