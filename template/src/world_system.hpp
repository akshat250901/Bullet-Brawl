#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "game_state_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();
	~WorldSystem();
	
	// starts the game
	GLFWwindow* init(RenderSystem* renderer, GameStateSystem* game_state_system, GLFWwindow* window);

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);

	bool paused;

private:
	// OpenGL window handle
	GLFWwindow* window;

	GameStateSystem* game_state_system;

	// restart level
	void restart_game();

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Respawns players
	void spawn_player(int player_num);

	// Game state
	RenderSystem* renderer;
	float current_speed;
	float next_turtle_spawn;
	float next_fish_spawn;
	Entity player;
	Entity player2;

	//Key states
	bool upKey;
	bool downKey;
	bool rightKey;
	bool leftKey;

	//Key states P2
	bool wUpKey;
	bool sDownKey;
	bool aLeftKey;
	bool dRightKey;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
