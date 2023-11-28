#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// A simple physics system that moves rigid bodies and checks for collision
class SoundSystem
{
public:
	void step(float elapsed_ms);

	// gun sounds
	Mix_Chunk* pistol_shoot_sound;
	Mix_Chunk* smg_shoot_sound;
	Mix_Chunk* ar_shoot_sound;
	Mix_Chunk* sniper_shoot_sound;
	Mix_Chunk* shotgun_shoot_sound;

	Mix_Chunk* player_reload_sound;
	Mix_Chunk* weapon_pickup_sound;

	Mix_Chunk* hit_sound;
	// player sounds
	Mix_Chunk* player_step_sound;
	Mix_Chunk* player_fall_sound;

	// misc sounds
	Mix_Chunk* powerup_pickup_sound;

	void init_sounds();

	// gun sounds
	void play_shoot_sound(std::string gun_type);
	void play_reload_sound(std::string gun_type);
	void play_pickup_sound(int type);
	void play_hit_sound();

	// misc sounds
	void play_walk_sound();

	void play_fall_sound();
};

