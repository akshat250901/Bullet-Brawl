
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>
#include <thread>
// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "animation_system.hpp"
#include "random_drops_system.hpp"
#include "game_state_system.hpp"
#include "main_menu_system.hpp"
#include "movement_system.hpp"
#include "gun_system.hpp"
#include "sound_system.hpp"
#include "out_of_bounds_arrow_system.hpp"
#include "player_respawn_system.hpp"
#include "world_init.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	GameStateSystem game_state_system;
	MainMenuSystem main_menu_system;
	WorldSystem world_system;
	RenderSystem render_system;
	PhysicsSystem physics_system;
	AnimationSystem animation_system;
	RandomDropsSystem random_drops_system(&render_system);
	MovementSystem movement_system;
	SoundSystem sound_system;
	GunSystem gun_system(&render_system, &sound_system);
	OutOfBoundsArrowSystem out_of_bounds_arrow_system;
	PlayerRespawnSystem player_respawn_system(&render_system, &game_state_system);

	// Initializing window
	GLFWwindow* window = game_state_system.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	sound_system.init_sounds();
	render_system.init(window);
	main_menu_system.initialize_main_menu(&render_system, &game_state_system, window);
	

	// variable timestep loop
	auto t = Clock::now();
	while (!game_state_system.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;
		if (game_state_system.get_current_state() == GameStateSystem::GameState::Winner) {
			createDeathScreen(&render_system, &game_state_system, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });
			game_state_system.set_winner(-1);
			game_state_system.change_game_state(0);
			render_system.draw();
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
		if (game_state_system.get_current_state() == 0 || game_state_system.get_current_state() == 1) {
			if (game_state_system.is_state_changed) {
				main_menu_system.initialize_main_menu(&render_system, &game_state_system, window);
				game_state_system.is_state_changed = false;
			}
		} else if (game_state_system.get_current_state() == 2 || game_state_system.get_current_state() == 3) {
			if (game_state_system.is_state_changed) {
				world_system.init(&render_system, &game_state_system, window, &sound_system);
				game_state_system.is_state_changed = false;
			}
			if (!world_system.paused) {
				movement_system.step(elapsed_ms);
				out_of_bounds_arrow_system.step();
				gun_system.step(elapsed_ms);
				world_system.step(elapsed_ms);
				physics_system.step(elapsed_ms);
				random_drops_system.step(elapsed_ms);
				animation_system.step(elapsed_ms);
				sound_system.step(elapsed_ms);
				world_system.handle_collisions();
				player_respawn_system.step();
				random_drops_system.handleInterpolation(elapsed_ms);
			}
		}
		render_system.draw();
	}

	return EXIT_SUCCESS;
}
