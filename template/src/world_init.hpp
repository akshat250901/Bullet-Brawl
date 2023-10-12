#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
Entity createPlayer2(RenderSystem* renderer, vec2 pos);
// the platform
Entity createPlatform(RenderSystem* renderer, vec3 color, vec2 position, vec2 size);
// the enemy
Entity createTurtle(RenderSystem* renderer, vec2 position);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// the background
Entity createBackground(RenderSystem* renderer, vec3 color, vec2 position, vec2 size);
// a pebble
Entity createPebble(vec2 pos, vec2 size);
// a bullet
Entity createBullet(bool isProjectile, vec2 pos, Entity& player);

