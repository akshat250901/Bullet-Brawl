#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "game_state_system.hpp"
#include <random>
#include "sound_system.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PlayerRespawnSystem
{
public:
	void step();

	PlayerRespawnSystem(RenderSystem* renderer, GameStateSystem* gameStateSystem, SoundSystem* sound_system);

private:
    RenderSystem* renderer;
    GameStateSystem* game_state_system;
    SoundSystem* sound_system;

    std::default_random_engine rng;
    std::uniform_int_distribution<int> uniform_dist_int; // number between 0..1
};