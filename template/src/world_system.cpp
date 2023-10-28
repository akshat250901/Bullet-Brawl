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
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}


GLFWwindow* WorldSystem::init(RenderSystem* renderer_arg, GameStateSystem* game_state_system, GLFWwindow* window) {
	this->window = window;
	this->game_state_system = game_state_system;
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
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");
	paused = false;

	// Set all states to default
	restart_game();

	return window;
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
		}
		else {
			platform.collider_active_player1 = false;
		}

		if ((playerMotion2.position.y + playerMotion2.scale.y / 2.0f) <= (platformMotion.position.y - platformMotion.scale.y / 2.0f)) {
			platform.collider_active_player2 = true;
		}
		else {
			platform.collider_active_player2 = false;
		}
	}

	// check if players are out of window
	if (playerMotion.position.y > window_height_px + 50) {
		// Player death logic
		playerMotion.position = vec2(900, 300);
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
		playerMotion2.position = vec2(300, 200);
		playerMotion2.velocity = vec2(0, 0);

		// Set timer to 0 for all power ups to stats are reset

		PlayerStatModifier& PlayerStatModifier = registry.playerStatModifiers.get(player2);

		for (auto& kv : PlayerStatModifier.powerUpStatModifiers) {
			kv.second.timer_ms = 0;
		}
	}

	// Decrement timers in the PlayerStatModifier
	for (Entity playerEntity : registry.players.entities) {
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
		else {
			parallaxBackground.scrollingSpeed = 0.0f;
		}
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();

	upKey = false;
	downKey = false;
	rightKey = false;
	leftKey = false;

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
	createBackgroundIsland(renderer, game_state_system, { window_width_px / 2, window_height_px / 2 }, { window_width_px, window_height_px });

	// Create players
	player2 = spawn_player({ 300, 200 }, { 1.f, 0, 0 });
	player = spawn_player({ 900, 300 }, { 0, 1.f, 0 });

	// Create platforms
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 390, 130 }, { 320, 10 }); // Top
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 415, 220 }, { 470, 10 }); // Second Top
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 470, 310 }, { 616, 10 }); // Third Top
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 530, 415 }, { 800, 10 }); // Third Top
	createPlatform(renderer, { 255.0f, 0.1f, 0.1f }, { 590, 530 }, { 1011, 10 }); // bottom platform

}

Entity WorldSystem::spawn_player(vec2 player_location, vec3 player_color) {
	auto player = createPlayer(renderer, player_location);
	registry.players.get(player).color = player_color;
	registry.players.get(player).healths = healths;

	return player;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {

	handle_player_platform_collisions();
	handle_player_powerup_collisions();
	handle_player_bullet_collisions();

}

void WorldSystem::handle_player_platform_collisions() {
	auto& playerPlatformCollisionsRegistry = registry.playerPlatformCollisions;
	// Flag to check if there are no player-platform collisions
	bool noPlayer1PlatformCollisions = true;
	bool noPlayer2PlatformCollisions = true;

	// Loop over all player platform collisions
	for (uint i = 0; i < playerPlatformCollisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = playerPlatformCollisionsRegistry.entities[i];
		Entity entity_other = playerPlatformCollisionsRegistry.components[i].other_entity;

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

	// Remove all collisions from player-platform
	registry.playerPlatformCollisions.clear();
}

void WorldSystem::handle_player_powerup_collisions() {
	auto& playerPowerUpCollisionsRegistry = registry.playerPowerUpCollisions;
	// Loop over all player powerup collisions
	for (uint i = 0; i < playerPowerUpCollisionsRegistry.components.size(); i++) {

		// The entity and its collider
		Entity entity = playerPowerUpCollisionsRegistry.entities[i];
		Entity entity_other = playerPowerUpCollisionsRegistry.components[i].other_entity;

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
			}
			else {
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
	// Remove all collisions from player-powerup
	registry.playerPowerUpCollisions.clear();
}

void WorldSystem::handle_player_bullet_collisions() {
	auto& playerBulletCollisionRegistry = registry.playerBulletCollisions;
	// Loop over all player powerup collisions
	for (uint i = 0; i < playerBulletCollisionRegistry.components.size(); i++) {

		// The entity and its collider
		Entity entity = playerBulletCollisionRegistry.entities[i];
		Entity entity_other = playerBulletCollisionRegistry.components[i].other_entity;

		// Player-bullet collisions
		if (registry.players.has(entity) && registry.bullets.has(entity_other)) {
			Player& hit_player = registry.players.get(entity);
			Motion& playerMotion = registry.motions.get(entity);

			Motion& bullet_motion = registry.motions.get(entity_other);

			std::uniform_int_distribution<int> distribution(300, 900);
			std::uniform_int_distribution<int> distribution_y(50, 400);
			playerMotion.position = vec2(distribution(rng), distribution_y(rng));
			playerMotion.velocity = vec2(0, 0);
			auto health_container = registry.healths;
			Entity to_remove;
			for (int i = 0; i < health_container.components.size(); i++) {
				Health health_entity = health_container.components[i];
				if (health_entity.player == entity) {					
					registry.renderRequests.remove(health_container.entities[i]);
					registry.healths.remove(health_container.entities[i]);
					hit_player.healths = hit_player.healths - 1;
					if (hit_player.healths == 0) {
						// TODO: Add a screen to show which player won
						restart_game();
					}
					break;
				}
			}

			registry.remove_all_components_of(entity_other);
		}
	}
	// Remove all collisions from player-bullet
	registry.playerBulletCollisions.clear();
}


// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		// paused = !paused;
		while (registry.bullets.entities.size() > 0)
	    	registry.remove_all_components_of(registry.bullets.entities.back());
		while (registry.powerUps.entities.size() > 0)
	    	registry.remove_all_components_of(registry.powerUps.entities.back());
		game_state_system->change_game_state(0);
	}

	if (!paused) {

		Player& player_object = registry.players.get(player);
		Player& player_object_2 = registry.players.get(player2);

		Controller& player_controller = registry.controllers.get(player);
		Controller& player_2_controller = registry.controllers.get(player2);

		if (key == GLFW_KEY_SEMICOLON && action == GLFW_PRESS) {
			Motion& player_motion = registry.motions.get(player);

			Entity bullet = createBullet(renderer, true, vec2(player_motion.position.x, player_motion.position.y), player);
			Motion& bullet_motion = registry.motions.get(bullet);
		}
		else if (key == GLFW_KEY_APOSTROPHE && action == GLFW_PRESS) {
			Motion& player_motion = registry.motions.get(player);

			Entity bullet = createBullet(renderer, false, vec2(player_motion.position.x, player_motion.position.y), player);
			Motion& bullet_motion = registry.motions.get(bullet);
			player_object.is_shooting = true;
		}
		else if (key == GLFW_KEY_APOSTROPHE && (action == GLFW_RELEASE || action == GLFW_REPEAT)) {
			player_object.is_shooting = false;
		}

		// Key handler for arrow keys
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
			player_controller.rightKey = true;
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			player_controller.leftKey = true;
		}
		else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
			player_controller.rightKey = false;
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
			player_controller.leftKey = false;
		}


		if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
			player_controller.upKey = true;
		}
		else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
			player_controller.downKey = true;
		}
		else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
			player_controller.upKey = false;
		}
		else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
			player_controller.downKey = false;
		}

		// Key handler for player 2 keys

		if (game_state_system->get_current_state() == 1) {
			if (key == GLFW_KEY_G && action == GLFW_PRESS) {
				Motion& player_motion = registry.motions.get(player2);

				Entity bullet = createBullet(renderer, true, vec2(player_motion.position.x, player_motion.position.y), player2);
				Motion& bullet_motion = registry.motions.get(bullet);
			}
			else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
				Motion& player_motion = registry.motions.get(player2);

				Entity bullet = createBullet(renderer, false, vec2(player_motion.position.x, player_motion.position.y), player2);
				Motion& bullet_motion = registry.motions.get(bullet);
				player_object_2.is_shooting = true;
			}
			else if (key == GLFW_KEY_H && (action == GLFW_RELEASE || GLFW_REPEAT)) {
				player_object_2.is_shooting = false;
			}

			if (key == GLFW_KEY_D && action == GLFW_PRESS) {
				player_2_controller.rightKey = true;
			}
			else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
				player_2_controller.leftKey = true;
			}
			else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
				player_2_controller.rightKey = false;
			}
			else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
				player_2_controller.leftKey = false;
			}

			if (key == GLFW_KEY_W && action == GLFW_PRESS) {
				player_2_controller.upKey = true;
			}
			else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
				player_2_controller.downKey = true;
			}
			else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
				player_2_controller.upKey = false;
			}
			else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
				player_2_controller.downKey = false;
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

}
void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(vec2)mouse_position; // dummy to avoid compiler warning
}
