// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"

// Game configuration
const size_t MAX_TURTLES = 15;
const size_t MAX_FISH = 5;
const size_t TURTLE_DELAY_MS = 2000 * 3;
const size_t FISH_DELAY_MS = 5000 * 3;

// Create the fish world
WorldSystem::WorldSystem()
	: points(0)
	, next_turtle_spawn(0.f)
	, next_fish_spawn(0.f)
	, upKey(false)
	, downKey(false)
	, rightKey(false)
	, leftKey(false) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Salmon Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("salmon_eat.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("salmon_dead.wav").c_str(),
			audio_path("salmon_eat.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Enable and disable platform colliders based on player position
	Motion& playerMotion = registry.motions.get(player);
	for (Entity entity : registry.platforms.entities) {
		Motion& platformMotion = registry.motions.get(entity);
		Platform& platform = registry.platforms.get(entity);

		if ((playerMotion.position.y + playerMotion.scale.y / 2.0f) <= (platformMotion.position.y - platformMotion.scale.y / 2.0f) ) {
			platform.colliderActive = true;
		} else {
			platform.colliderActive = false;
		}
	}

	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_timer_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& timer = registry.deathTimers.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if(timer.timer_ms < min_timer_ms){
			min_timer_ms = timer.timer_ms;
		}

		// restart the game once the death timer expired
		if (timer.timer_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.screen_darken_factor = 0;
            restart_game();
			return true;
		}
	}
	// reduce window brightness if any of the present salmons is dying
	screen.screen_darken_factor = 1 - min_timer_ms / 3000;

	// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the death timer

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Create a new player
	player = createPlayer(renderer, { 500, 200 });
	// registry.colors.insert(player, {1, 0.8f, 0.8f});

	// Create platforms
	createPlatform(renderer, { 0.1f, 0.1f, 0.1f }, { 600, 400 }, { 500, 20 }); // bottom platform
	createPlatform(renderer, { 0.1f, 0.1f, 0.1f }, { 600, 200 }, { 200, 20 }); // top platform
	createPlatform(renderer, { 0.1f, 0.1f, 0.1f }, { 900, 300 }, { 200, 20 }); // top left
	createPlatform(renderer, { 0.1f, 0.1f, 0.1f }, { 300, 300 }, { 200, 20 }); // top right

}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;

	// Flag to check if there are no player-platform collisions
	bool noPlayerPlatformCollisions = true;

	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;

		// Player platform collisions
		if (registry.players.has(entity)) {
			if (registry.platforms.has(entity_other)) {
				Motion& playerMotion = registry.motions.get(entity);
				Motion& platformMotion = registry.motions.get(entity_other);
				Platform& platform = registry.platforms.get(entity_other);

				// Player is touching the platform
				Player& player = registry.players.get(entity);

				if (playerMotion.velocity.y > 0 && platform.colliderActive) {
					playerMotion.position.y = platformMotion.position.y - (platformMotion.scale.y / 2.0f) - (playerMotion.scale.y / 2.0f);
					player.is_grounded = true;
					player.jump_remaining = 1;
					playerMotion.velocity.y = 0;

					// There is a player platform collision
					noPlayerPlatformCollisions = false;
				}
			}
		}
	}


	Player& player_object = registry.players.get(player);

	// if there are no player platform collisions, the player is not grounded
	if (noPlayerPlatformCollisions) {
		player_object.is_grounded = false;
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// Key handler for arrow keys
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		rightKey = true;
	} else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		leftKey = true;
	} else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
		rightKey = false;
	} else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
		leftKey = false;
	}
	

			
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		upKey = true;
	} else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		downKey = true;
	} else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
		upKey = false;
	} else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
		downKey = false;
	}

	Player& player_object = registry.players.get(player);
	
	if (!registry.deathTimers.has(player)) {
		Motion& playerMotion = registry.motions.get(player);
		//Handle inputs for left and right arrow keys

		if (rightKey && !leftKey) {
			playerMotion.velocity.x = player_object.speed;
			player_object.is_running_right = true;
		} else if (!rightKey && leftKey) {
			playerMotion.velocity.x = -player_object.speed;
			player_object.is_running_left = true;
		} else if (rightKey && leftKey) {
			playerMotion.velocity.x = 0;
			player_object.is_running_left = false;
			player_object.is_running_right = false;
		} else if (!rightKey && !leftKey) {
			player_object.is_running_left = false;
			player_object.is_running_right = false;
		}

		if (upKey) {
			if (player_object.is_grounded) {
				playerMotion.velocity.y = -player_object.jump_force;
			}
			else if (player_object.jump_remaining > 0) {
				playerMotion.velocity.y = -player_object.jump_force;
				player_object.jump_remaining--;
			}
		}

		if (downKey) {
			if (player_object.is_grounded) {
				playerMotion.position.y += 1.0f;
			}
		}
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(vec2)mouse_position; // dummy to avoid compiler warning
}
