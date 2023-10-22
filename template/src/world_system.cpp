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
	, leftKey(false)
	, wUpKey(false)
	, dRightKey(false)
	, aLeftKey(false)
	, sDownKey(false) {
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
	window = glfwCreateWindow(window_width_px, window_height_px, "Bullet Brawl", nullptr, nullptr);
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
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motion_container = registry.motions;

	// Remove entities that leave the screen
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i) {
		Motion& motion = motion_container.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f || motion.position.x + abs(motion.scale.x) > window_width_px || 
			motion.position.y + abs(motion.scale.y) < 0.f || motion.position.y + abs(motion.scale.y) > window_height_px) {
			if (!registry.players.has(motion_container.entities[i]) && registry.bullets.has(motion_container.entities[i])) // removing only bullets
				registry.remove_all_components_of(motion_container.entities[i]);
		}
	}

	// Enable and disable platform colliders based on player position
	Motion& playerMotion = registry.motions.get(player);
	Motion& playerMotion2 = registry.motions.get(player2);

	for (Entity entity : registry.platforms.entities) {
		Motion& platformMotion = registry.motions.get(entity);
		Platform& platform = registry.platforms.get(entity);

		if ((playerMotion.position.y + playerMotion.scale.y / 2.0f) <= (platformMotion.position.y - platformMotion.scale.y / 2.0f)) {
			platform.collider_active_player1 = true;
		} else {
			platform.collider_active_player1 = false;
		}

		if ((playerMotion2.position.y + playerMotion2.scale.y / 2.0f) <= (platformMotion.position.y - platformMotion.scale.y / 2.0f)) {
			platform.collider_active_player2 = true;
		} else {
			platform.collider_active_player2 = false;
		}
	}

	// check if players are out of window
	if (playerMotion.position.y > window_height_px + 50) {
		// Player death logic
		playerMotion.position = vec2( 500, 200);
		playerMotion.velocity = vec2(0, 0);


		// Set timer to 0 for all power ups to stats are reset

		PlayerStatModifier& PlayerStatModifier = registry.playerStatModifiers.get(player);
		
		for (auto& kv : PlayerStatModifier.powerUpStatModifiers) {
    		kv.second.timer_ms = 0;
		}

	}

	// check if players are out of window
	if (playerMotion2.position.y > window_height_px) {
		// Player death logic
		playerMotion2.position = vec2(700, 200);
		playerMotion2.velocity = vec2(0, 0);

		// Set timer to 0 for all power ups to stats are reset

		PlayerStatModifier& PlayerStatModifier = registry.playerStatModifiers.get(player2);
		
		for (auto& kv : PlayerStatModifier.powerUpStatModifiers) {
    		kv.second.timer_ms = 0;
		}
	}

	// Decrement timers in the PlayerStatModifier
	for (Entity playerEntity: registry.players.entities) {
		PlayerStatModifier& playerStatModifier = registry.playerStatModifiers.get(playerEntity);

		auto& powerUpMap = playerStatModifier.powerUpStatModifiers;

		for (auto it = powerUpMap.begin(); it != powerUpMap.end();) {
			StatModifier& statModifier = it->second;

			if (statModifier.hasTimer) {
				statModifier.timer_ms -= elapsed_ms_since_last_update;

				if (statModifier.timer_ms <= 0) {
					Player& currPlayer = registry.players.get(playerEntity);

					currPlayer.max_jumps -= statModifier.extra_jumps;
					currPlayer.jump_force /= statModifier.jump_force_modifier;
					currPlayer.running_force /= statModifier.running_force_modifier;
					currPlayer.speed /= statModifier.max_speed_modifier;

					it = powerUpMap.erase(it);
					
					continue;
				}
			}

			it++;
		}
	}

	//parallax background
	for (Entity backgroundEntity : registry.parallaxes.entities) {
		ParallaxBackground& parallaxBackground = registry.parallaxes.get(backgroundEntity);
		// Update the offset based on the speed and elapsed time.
		if (parallaxBackground.scrollingSpeedBack != 0.0f) {
			parallaxBackground.scrollingSpeed += parallaxBackground.scrollingSpeedBack * (elapsed_ms_since_last_update / 1000);
		}
		else if (parallaxBackground.scrollingSpeedMiddle != 0.0f) {
			parallaxBackground.scrollingSpeed += parallaxBackground.scrollingSpeedMiddle * (elapsed_ms_since_last_update / 1000);
		}
		else if (parallaxBackground.scrollingSpeedFront != 0.0f) {
			parallaxBackground.scrollingSpeed += parallaxBackground.scrollingSpeedFront * (elapsed_ms_since_last_update / 1000);
		}
	}

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

	//make background parallax
	createBackgroundBack(renderer, { window_width_px / 2, window_height_px / 2 }, { window_width_px + 200, window_height_px });
	createBackgroundMiddle(renderer, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });
	createBackgroundForeground(renderer, { window_width_px / 2,window_height_px / 2 }, { window_width_px, window_height_px });


	// Create players
	spawn_player(-1);

	// Create platforms
	createPlatform(renderer, { 0.1f, 0.1f, 0.1f }, { 600, 400 }, { 500, 20 }); // bottom platform
	createPlatform(renderer, { 0.1f, 0.1f, 0.1f }, { 600, 200 }, { 200, 20 }); // top platform
	createPlatform(renderer, { 0.1f, 0.1f, 0.1f }, { 900, 300 }, { 200, 20 }); // top left
	createPlatform(renderer, { 0.1f, 0.1f, 0.1f }, { 300, 300 }, { 200, 20 }); // top right

}

void WorldSystem::spawn_player(int player_num) {
	if (player_num == -1) {
		player = createPlayer(renderer, { 500, 200 });
		player2 = createPlayer2(renderer, { 700, 200 });
	}
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;

	// Flag to check if there are no player-platform collisions
	bool noPlayer1PlatformCollisions = true;
	bool noPlayer2PlatformCollisions = true;

	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;

		// Player platform collisions
		if (registry.players.has(entity) && registry.platforms.has(entity_other)) {
			
			Motion& playerMotion = registry.motions.get(entity);
			Motion& platformMotion = registry.motions.get(entity_other);
			Platform& platform = registry.platforms.get(entity_other);

			// Player model intersects the platform
			Player& curr_player = registry.players.get(entity);

			bool players_collider_active = platform.collider_active_player1;

			if (entity == player2) {
				players_collider_active = platform.collider_active_player2;
			}

			if (playerMotion.velocity.y > 0 && players_collider_active) {
				playerMotion.position.y = platformMotion.position.y - (platformMotion.scale.y / 2.0f) - (playerMotion.scale.y / 2.0f);
				curr_player.is_grounded = true;
				curr_player.jump_remaining = curr_player.max_jumps;
				playerMotion.velocity.y = 0;

				if (entity == player) {
					noPlayer1PlatformCollisions = false;
				}
				else if (entity == player2) {
					noPlayer2PlatformCollisions = false;
				}
			}
		}

		// Player powerup collisions
		if (registry.players.has(entity) && registry.powerUps.has(entity_other)) {
			Player& player = registry.players.get(entity);
			Motion& playerMotion = registry.motions.get(entity);
			PowerUp& powerUp = registry.powerUps.get(entity_other);
			PlayerStatModifier& playerStatModifier = registry.playerStatModifiers.get(entity);
			StatModifier& statModifier = powerUp.statModifier;

			if (playerStatModifier.powerUpStatModifiers.find(statModifier.name) != playerStatModifier.powerUpStatModifiers.end()) {
				//if player has powerup, reset the timer of the powerup
				StatModifier& modifier = playerStatModifier.powerUpStatModifiers.at(statModifier.name);
				modifier.timer_ms = 5000;
			} else { 
				// if player does not have power up, modify players stats and add to powerup map
				playerStatModifier.powerUpStatModifiers[statModifier.name] = statModifier;

				player.max_jumps += statModifier.extra_jumps;
				player.jump_force *= statModifier.jump_force_modifier;
				player.running_force *= statModifier.running_force_modifier;
				player.speed *= statModifier.max_speed_modifier;
			}

			registry.remove_all_components_of(entity_other);
		}
	}


	Player& player_object = registry.players.get(player);

	// if there are no player platform collisions, the player is not grounded
	if (noPlayer1PlatformCollisions) {
		player_object.is_grounded = false;
	}

	Player& player2_object = registry.players.get(player2);

	// if there are no player platform collisions, the player 2 is not grounded
	if (noPlayer2PlatformCollisions) {
		player2_object.is_grounded = false;
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

	Player& player_object = registry.players.get(player);
	Player& player_object_2 = registry.players.get(player2); 

	if (key == GLFW_KEY_SEMICOLON && action == GLFW_PRESS) {
		Motion& player_motion = registry.motions.get(player);

		Entity bullet = createBullet(true, vec2(player_motion.position.x, player_motion.position.y), player);
		Motion& bullet_motion = registry.motions.get(bullet);
	}
	else if (key == GLFW_KEY_APOSTROPHE && action == GLFW_PRESS) {
		Motion& player_motion = registry.motions.get(player);

		Entity bullet = createBullet(false, vec2(player_motion.position.x, player_motion.position.y), player);
		Motion& bullet_motion = registry.motions.get(bullet);
		player_object.is_shooting = true;
	}else if (key == GLFW_KEY_APOSTROPHE && (action == GLFW_RELEASE || action == GLFW_REPEAT)) {
		player_object.is_shooting = false;
	}

	// Key handler for arrow keys
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		rightKey = true;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		leftKey = true;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
		rightKey = false;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
		leftKey = false;
	}



	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		upKey = true;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		downKey = true;
	}
	else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
		upKey = false;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
		downKey = false;
	}

	// Key handler for player 2 keys

	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		Motion& player_motion = registry.motions.get(player2);

		Entity bullet = createBullet(true, vec2(player_motion.position.x, player_motion.position.y), player2);
		Motion& bullet_motion = registry.motions.get(bullet);
	}
	else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
		Motion& player_motion = registry.motions.get(player2);

		Entity bullet = createBullet(false, vec2(player_motion.position.x, player_motion.position.y), player2);
		Motion& bullet_motion = registry.motions.get(bullet);
		player_object_2.is_shooting = true;
	}
	else if (key == GLFW_KEY_H && (action == GLFW_RELEASE || GLFW_REPEAT)) {
		player_object_2.is_shooting = false;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		dRightKey = true;
	}
	else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		aLeftKey = true;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		dRightKey = false;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		aLeftKey = false;
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		wUpKey = true;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		sDownKey = true;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		wUpKey = false;
	}
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		sDownKey = false;
	}


	Motion& playerMotion = registry.motions.get(player);

	//Handle inputs for left and right arrow keys
	if (rightKey && !leftKey) {
		player_object.is_running_right = true;
		player_object.facing_right = true;
	}
	else if (!rightKey && leftKey) {
		player_object.is_running_left = true;
		player_object.facing_right = false;
	}
	else if ((!rightKey && !leftKey) || (rightKey && leftKey)) {
		player_object.is_running_left = false;
		player_object.is_running_right = false;
	}

	// Handle up arrow input for jumping
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


	
	

	Player& player2_object = registry.players.get(player2);
	Motion& playerMotion2 = registry.motions.get(player2);

	//Handle inputs for left and right arrow keys
	if (dRightKey && !aLeftKey) {
		player2_object.is_running_right = true;
		player2_object.facing_right = true;
	}
	else if (!dRightKey && aLeftKey) {
		player2_object.is_running_left = true;
		player2_object.facing_right = false;
	}
	else if ((!dRightKey && !aLeftKey) || (dRightKey && aLeftKey)) {
		player2_object.is_running_left = false;
		player2_object.is_running_right = false;
	}

	// Handle up arrow input for jumping
	if (wUpKey) {
		if (player2_object.is_grounded) {
			playerMotion2.velocity.y = -player2_object.jump_force;
		}
		else if (player2_object.jump_remaining > 0) {
			playerMotion2.velocity.y = -player2_object.jump_force;
			player2_object.jump_remaining--;
		}
	}

	if (sDownKey) {
		if (player2_object.is_grounded) {
			playerMotion2.position.y += 1.0f;
		}
	}
	

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_Q) {
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
