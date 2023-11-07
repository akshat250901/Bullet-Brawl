#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "game_state_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;
// the player
Entity createPlayer(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 pos);
// the platform
Entity createPlatform(RenderSystem* renderer, vec3 color, vec2 position, vec2 size);
// a bullet
Entity createBullet(RenderSystem* renderer, Entity gunEntity);
// a projectile
Entity createProjectile(RenderSystem* renderer, bool isProjectile, vec2 pos, Entity& player);
// a powerup
Entity createPowerup(RenderSystem* renderSystem, vec2 pos, vec2 scale, vec3 ColoredVertex);
// a gun mystery box
Entity createGunMysteryBox(RenderSystem* renderSystem, vec2 pos, vec2 scale);
// create a gun
Entity createGun(RenderSystem* renderSystem, vec2 scale, std::string gun);
//Island
Entity createBackgroundIsland(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 position, vec2 size);
// a background back layer
Entity createBackgroundBack(RenderSystem* renderer, vec2 position, vec2 size);
// a background middle layer
Entity createBackgroundMiddle(RenderSystem* renderer, vec2 position, vec2 size);
// a background
Entity createBackgroundForeground(RenderSystem* renderer, vec2 position, vec2 size);
Entity createGun(RenderSystem* renderSystem, vec2 scale);
// render space map
Entity createBackgroundSpace(RenderSystem* renderer, GameStateSystem* game_state_system, vec2 position, vec2 size);
// render island map
void createIslandMap(RenderSystem* renderer, GameStateSystem* game_state_system, int window_width_px, int window_height_px);
// render jungle map
void createJungleMap(RenderSystem* renderer, GameStateSystem* game_state_system, int window_width_px, int window_height_px);
// render space map
void createSpaceMap(RenderSystem* renderer, GameStateSystem* game_state_system, int window_width_px, int window_height_px);

