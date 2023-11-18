// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include "physics_system.hpp"
#include "stat_util.cpp"
#include "create_gun_util.cpp"

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


GLFWwindow* WorldSystem::init(RenderSystem* renderer_arg, GameStateSystem* game_state_system, GLFWwindow* window, SoundSystem* sound_system) {
	this->window = window;
	this->renderer = renderer_arg;
	this->game_state_system = game_state_system;
	this->sound_system = sound_system;
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
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
			motion.position.y + abs(motion.scale.y) > window_height_px) {
			if (!registry.players.has(motion_container.entities[i]) && (registry.bullets.has(motion_container.entities[i]) || registry.nonInteractables.has(motion_container.entities[i]))) {
				registry.remove_all_components_of(motion_container.entities[i]);
			}
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

	const float kill_limit = 800.0f;

	// check if players are out of window
	if (playerMotion.position.y > window_height_px + abs(playerMotion.scale.y / 2) +  kill_limit) {
		// Player death logic
		playerMotion.position = vec2(900, 300);
		playerMotion.velocity = vec2(0, 0);

		CreateGunUtil::givePlayerStartingPistol(renderer, player, true);

		if (game_state_system->get_current_state() == 2) {
			Player& hit_player = registry.players.get(player);
			auto health_container = registry.lives;
			for (int i = 0; i < health_container.components.size(); i++) {
				Life health_entity = health_container.components[i];
				if (health_entity.player == player) {
					registry.renderRequests.remove(health_container.entities[i]);
					registry.lives.remove(health_container.entities[i]);
					hit_player.lives = hit_player.lives - 1;
					if (hit_player.lives == 0) {
						game_state_system->set_winner(1);
						game_state_system->change_game_state(GameStateSystem::GameState::Winner);
						restart_game();
					}
					break;
				}
			}
		}


		// Set timer to 0 for all power ups to stats are reset

		PlayerStatModifier& PlayerStatModifier = registry.playerStatModifiers.get(player);

		for (auto& kv : PlayerStatModifier.powerUpStatModifiers) {
			kv.second.timer_ms = 0;
		}

	}

	// check if players are out of window
	if (playerMotion2.position.y > window_height_px + abs(playerMotion2.scale.y / 2) + kill_limit) {
		// Player death logic
		playerMotion2.position = vec2(300, 200);
		playerMotion2.velocity = vec2(0, 0);

		CreateGunUtil::givePlayerStartingPistol(renderer, player2, true);

		Player& hit_player = registry.players.get(player2);
		auto health_container = registry.lives;
		for (int i = 0; i < health_container.components.size(); i++) {
			Life health_entity = health_container.components[i];
			if (health_entity.player == player2) {
				registry.renderRequests.remove(health_container.entities[i]);
				registry.lives.remove(health_container.entities[i]);
				hit_player.lives = hit_player.lives - 1;
				if (hit_player.lives == 0) {
					game_state_system->set_winner(2);
					game_state_system->change_game_state(GameStateSystem::GameState::Winner);
					restart_game();
				}
				break;
			}
		}

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

					StatUtil::remove_stat_modifier(currPlayer, statModifier);

					it = powerUpMap.erase(it);

					continue;
				}
			}

			it++;
		}
	}

	// deal with popups
	for (Entity popup_entity : registry.popupIndicator.entities) {
		PopupIndicator& popup = registry.popupIndicator.get(popup_entity);
		Motion& popup_motion = registry.motions.get(popup_entity);

		Player& player = registry.players.get(popup.player);
		Motion& player_motion = registry.motions.get(popup.player);

		if (popup.type == "Reload") {
			for (Gun gun : registry.guns.components) {
				if (gun.currentlyReloading && gun.gunOwner == popup.player) {
					popup_motion.position = { player_motion.position.x , player_motion.position.y - 50 };
				}
				else if (!gun.currentlyReloading && gun.gunOwner == popup.player) {
					registry.popupIndicator.remove(popup_entity);
					registry.motions.remove(popup_entity);
				}
			}
		}
		else {
			if (popup.timer > 0) {
				popup.timer -= elapsed_ms_since_last_update;

				if (popup.timer < 800.f) {
					popup_motion.position = { player_motion.position.x , (player_motion.position.y - 30) - (1000 - popup.timer) / 10 };
				}
				else {
					popup_motion.position = { player_motion.position.x , player_motion.position.y - 50 };
				}
			}
			else {
				registry.popupIndicator.remove(popup_entity);
				registry.motions.remove(popup_entity);
			}
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

	// TODO: USE ISLAND MAP FOR TUTORIAL
	// ISLAND MAP
	if (game_state_system->get_current_state() == 3) {
		createIslandMap(renderer, game_state_system, window_width_px, window_height_px);
	} else if (game_state_system->get_current_state() == 2) {
		int level = game_state_system->get_current_level();
		if (level == 1) {
			createJungleMap(renderer, game_state_system, window_width_px, window_height_px);
		} else if (level == 2) {
			createSpaceMap(renderer, game_state_system, window_width_px, window_height_px);
		} else if (level == 3) {
			createTempleMap(renderer, game_state_system, window_width_px, window_height_px);
		}
	}

	Keybinds player2_keys{
		GLFW_KEY_UP,
		GLFW_KEY_DOWN,
		GLFW_KEY_LEFT,
		GLFW_KEY_RIGHT,
		GLFW_KEY_SEMICOLON,
		GLFW_KEY_APOSTROPHE,
	};
	Keybinds player1_keys{
	GLFW_KEY_W,
	GLFW_KEY_S,
	GLFW_KEY_A,
	GLFW_KEY_D,
	GLFW_KEY_G,
	GLFW_KEY_H,
	};
	player2 = spawn_player({ 300, 200 }, { 1.f, 0, 0 }, player1_keys);
	player = spawn_player({ 900, 300 }, { 0, 1.f, 0 }, player2_keys);

	createOutOfBoundsArrow( renderer, player, true);
	createOutOfBoundsArrow( renderer, player2, false);

	// Add default pistols for players

	CreateGunUtil::givePlayerStartingPistol(renderer, player, false);
	CreateGunUtil::givePlayerStartingPistol(renderer, player2, false);
}

Entity WorldSystem::spawn_player(vec2 player_location, vec3 player_color, Keybinds keybinds) {
	auto player = createPlayer(renderer, game_state_system, player_location);
	registry.players.get(player).color = player_color;
	registry.players.get(player).lives = 5;
	registry.players.get(player).keybinds = keybinds;
	return player;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {

	handle_player_platform_collisions();
	handle_player_powerup_collisions();
	handle_player_bullet_collisions();
	handle_player_mystery_box_collisions();
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

			sound_system->play_pickup_sound(1);
			createPopupIndicator(renderer, powerUp.statModifier.name, entity);

			if (playerStatModifier.powerUpStatModifiers.find(statModifier.name) != playerStatModifier.powerUpStatModifiers.end()) {
				//if player has powerup, reset the timer of the powerup
				StatModifier& modifier = playerStatModifier.powerUpStatModifiers.at(statModifier.name);
				modifier.timer_ms = 5000;
			}
			else {
				// if player does not have power up, modify players stats and add to powerup map
				playerStatModifier.powerUpStatModifiers[statModifier.name] = statModifier;

				StatUtil::apply_stat_modifier(player, statModifier);
			}

			registry.remove_all_components_of(entity_other);
		}
	}
	// Remove all collisions from player-powerup
	registry.playerPowerUpCollisions.clear();
}

void WorldSystem::handle_player_bullet_collisions() {
	auto& playerBulletCollisionRegistry = registry.playerBulletCollisions;
	// Loop over all player bullet collisions
	for (uint i = 0; i < playerBulletCollisionRegistry.components.size(); i++) {

		// The entity and its collider
		Entity entity = playerBulletCollisionRegistry.entities[i];
		Entity entity_other = playerBulletCollisionRegistry.components[i].other_entity;

		// Player-bullet collisions
		if (registry.players.has(entity) && registry.bullets.has(entity_other)) {
			Player& hit_player = registry.players.get(entity);
			Motion& playerMotion = registry.motions.get(entity);
			Bullet& bullet = registry.bullets.get(entity_other);

			sound_system->play_hit_sound();

			if (bullet.isHitscan) {

				printf("HITSCAN KNOCKBACK: %f\n", bullet.knockback);
				playerMotion.velocity.x += bullet.knockback;

			} else {
				Motion& bullet_motion = registry.motions.get(entity_other);
				
				float distanceTravelled = abs(bullet_motion.position.x - bullet.originalXPosition);

				if (bullet.hasNormalDropOff) {
					float dropOffPenalty = distanceTravelled * 0.5 * bullet.distanceStrengthModifier;

					if (dropOffPenalty >= bullet.knockback) {
						dropOffPenalty = bullet.knockback;
					}

					float knockbackWithDropOff = bullet.knockback - dropOffPenalty;

					printf("KNOCKBACK WITH PENALTY: %f\n", knockbackWithDropOff);

					playerMotion.velocity.x += knockbackWithDropOff * (bullet_motion.velocity.x < 0 ? -1 : 1); 
				} else {
					float distanceBonus = distanceTravelled * bullet.distanceStrengthModifier;

					float knockbackWithBonus = bullet.knockback + distanceBonus;

					printf("KNOCKBACK WITH BONUS: %f\n", knockbackWithBonus);

					playerMotion.velocity.x += knockbackWithBonus * (bullet_motion.velocity.x < 0 ? -1 : 1); 
				}
			}


			registry.remove_all_components_of(entity_other);
		}
	}
	// Remove all collisions from player-bullet
	registry.playerBulletCollisions.clear();
}

void WorldSystem::handle_player_mystery_box_collisions() {
	auto& playerMysteryBoxCollisionRegistry = registry.playerMysteryBoxCollisions;
	// Loop over all player mystery box collisions
	for (uint i = 0; i < playerMysteryBoxCollisionRegistry.components.size(); i++) {

		// The entity and its collider
		Entity entity = playerMysteryBoxCollisionRegistry.entities[i];
		Entity entity_other = playerMysteryBoxCollisionRegistry.components[i].other_entity;

		// Player-mystery box collisions
		if (registry.players.has(entity) && registry.gunMysteryBoxes.has(entity_other)) {
			Player& hit_player = registry.players.get(entity);
			Motion& playerMotion = registry.motions.get(entity);
			
			GunMysteryBox& mystery_box = registry.gunMysteryBoxes.get(entity_other);
			Gun& randomGun = mystery_box.randomGun;

			sound_system->play_pickup_sound(0);
			createPopupIndicator(renderer ,randomGun.name, entity);

			// Iterate over all elements in guns to find gun owned by current player
			auto& gun_container = registry.guns;
			for (uint i = 0; i < gun_container.components.size(); i++) {
				Gun& gun_i = gun_container.components[i];
				Entity entity_i = gun_container.entities[i];

				Entity gun_owner = gun_i.gunOwner;

				if (gun_owner != entity) {
					continue;
				}

				// Remove old stat modifier and apply new ones
				StatModifier oldStatModifier = gun_i.statModifier;
				StatUtil::remove_stat_modifier(hit_player, oldStatModifier);

				// Remove old gun
				registry.remove_all_components_of(entity_i);

				// Give new gun to player
				StatModifier newStatModifier = randomGun.statModifier;
				StatUtil::apply_stat_modifier(hit_player, newStatModifier);

				Entity newGunEntity = createGun(renderer, randomGun.gunSize, randomGun.name);
				Gun& newGunComponent = gun_container.insert(newGunEntity, randomGun);
				newGunComponent.gunOwner = entity;
			}

			registry.remove_all_components_of(entity_other);
		}
	}
	// Remove all collisions from player-mystery box
	registry.playerMysteryBoxCollisions.clear();
}

void WorldSystem::handle_player(int key, int action, Entity player_to_handle)
{

	Player& player_object = registry.players.get(player_to_handle);

	Controller& player_controller = registry.controllers.get(player_to_handle);

	// Key handler for arrow keys
	if (key == player_object.keybinds.right && action == GLFW_PRESS) {
		player_controller.rightKey = true;
	}
	else if (key == player_object.keybinds.left && action == GLFW_PRESS) {
		player_controller.leftKey = true;
	}
	else if (key == player_object.keybinds.right && action == GLFW_RELEASE) {
		player_controller.rightKey = false;
	}
	else if (key == player_object.keybinds.left && action == GLFW_RELEASE) {
		player_controller.leftKey = false;
	}


	if (key == player_object.keybinds.up && action == GLFW_PRESS) {
		player_controller.upKey = true;
	}
	else if (key == player_object.keybinds.down && action == GLFW_PRESS) {
		player_controller.downKey = true;
	}
	else if (key == player_object.keybinds.up && action == GLFW_RELEASE) {
		player_controller.upKey = false;
	}
	else if (key == player_object.keybinds.down && action == GLFW_RELEASE) {
		player_controller.downKey = false;
	}


	if (key == player_object.keybinds.projectile && action == GLFW_PRESS) {
		Motion& player_motion = registry.motions.get(player_to_handle);
		createProjectile(renderer, true, vec2(player_motion.position.x, player_motion.position.y), player_to_handle);
	}
	else if (key == player_object.keybinds.bullet && action == GLFW_PRESS) {
		player_controller.fireKey = true;
	}
	else if (key == player_object.keybinds.bullet && action == GLFW_RELEASE) {
		player_controller.fireKey = false;
	}
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

		handle_player(key, action, player);
		if (game_state_system->get_current_state() == 2) {
			handle_player(key, action, player2);
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
	(vec2)mouse_position; // dummy to avoid compiler warning
}


