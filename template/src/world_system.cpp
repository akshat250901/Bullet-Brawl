// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include "physics_system.hpp"
#include "stat_util.cpp"
#include "create_gun_util.cpp"

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


GLFWwindow* WorldSystem::init(RenderSystem* renderer_arg, GameStateSystem* game_state_system, GLFWwindow* window, SoundSystem* sound_system, RandomDropsSystem* random_drops_system) {
	this->window = window;
	this->renderer = renderer_arg;
	this->game_state_system = game_state_system;
	this->sound_system = sound_system;
	this->random_drops_system = random_drops_system;
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
	ScreenState& screen = registry.screenStates.components[0];
	int winner = -1;
	// Remove entities that leave the screen
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	float sideBoundaryOffset = 200;

	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i) {
		Motion& motion = motion_container.components[i];
		if (motion.position.x + abs(motion.scale.x) < -sideBoundaryOffset || motion.position.x + abs(motion.scale.x) > window_width_px + sideBoundaryOffset ||
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

	// Decrement timers in the PlayerStatModifier and Invincibility
	for (Entity playerEntity : registry.players.entities) {
		Player& currPlayer = registry.players.get(playerEntity);
		PlayerStatModifier& playerStatModifier = registry.playerStatModifiers.get(playerEntity);
		Invincibility& invincibility = registry.invincibility.get(playerEntity);

		auto& powerUpMap = playerStatModifier.powerUpStatModifiers;

		if (invincibility.has_TIMER) {
			invincibility.timer_ms -= elapsed_ms_since_last_update;
			int timer_quarter_sec = invincibility.timer_ms / 250.f;
			if ((timer_quarter_sec % 2) == 0) {
				currPlayer.color = invincibility.player_original_color;
			} else {
				currPlayer.color = invincibility.invincibility_color;
			}

			if (invincibility.timer_ms <= 0) {
				currPlayer.color = invincibility.player_original_color;
				invincibility.has_TIMER = false;
			}
		}

		for (auto it = powerUpMap.begin(); it != powerUpMap.end();) {
			StatModifier& statModifier = it->second;

			if (statModifier.hasTimer) {
				statModifier.timer_ms -= elapsed_ms_since_last_update;

				if (statModifier.timer_ms <= 0) {
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

	float min_timer_ms = 1000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& timer = registry.deathTimers.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < min_timer_ms) {
			min_timer_ms = timer.timer_ms;
		}
		
		if (timer.timer_ms < 0) {
			if (game_state_system->get_current_state() == 3) {
				registry.remove_all_components_of(entity);
				return false;
			}
			else {
				// restart the game once the death timer expired
				registry.deathTimers.remove(entity);
				screen.screen_darken_factor = 0;
				game_state_system->change_game_state(GameStateSystem::GameState::Winner);
				return true;
			}
		}
	}


	for (Entity entity : registry.texts.entities) {
		// progress timer
		Text& text = registry.texts.get(entity);
		if (text.tag == "PLAYER_FALL")
		{

			text.persist_timer_ms -= elapsed_ms_since_last_update;

			if (text.persist_timer_ms > 0) {
				continue;
			}

			text.timer_ms -= elapsed_ms_since_last_update;

			if (text.timer_ms > 0) {
				text.opacity = (text.timer_ms/text.total_fade_time);
			} else {
				registry.remove_all_components_of(entity);
			}
		}
		if (text.tag == "HEALTH_COUNT") {
			if (text.owner == player) {
				text.string = "LIVES: " + std::to_string(registry.players.get(player).lives);
			}
			else {
				text.string = "LIVES: " + std::to_string(registry.players.get(player2).lives);
			}
		}
	}

	if (game_state_system->get_current_state() != 3) {
		// reduce window brightness if any of the present salmons is dying
		screen.screen_darken_factor = 1 - min_timer_ms / 1000;
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
	//registry.list_all_components();

	// TODO: USE ISLAND MAP FOR TUTORIAL
	// ISLAND MAP
	if (game_state_system->get_current_state() == 3) {
		createTutorialMap(renderer, game_state_system, window_width_px, window_height_px);
		random_drops_system->is_tutorial_intialized = false;
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

	float textHorizontalOffset = 400.0f;
	float textVerticalOffset = 40.0f;
	float horizontalAlignment = 0;

	player2 = spawn_player({ 300, 200 }, { 1.f, 0, 0 }, player1_keys);
	createOutOfBoundsArrow( renderer, player2, false);
	CreateGunUtil::givePlayerStartingPistol(renderer, player2, false);

	if (game_state_system->get_current_state() == 3) {
		player = spawn_player({ 700, 200 }, { 1.f, 1.f, 1.f }, player2_keys);
		textHorizontalOffset = 600;
		horizontalAlignment = 1;
	} else {
		player = spawn_player({ 900, 300 }, { 0, 1.f, 0 }, player2_keys);
		createOutOfBoundsArrow(renderer, player, true);
		CreateGunUtil::givePlayerStartingPistol(renderer, player, false);

		//Create text for ammo counter and weapon
		createText("GREEN PLAYER", {window_width_px - textHorizontalOffset, window_height_px - textVerticalOffset - 40 }, {0.0f, 255.0f, 0.0f}, 2.5f, 1.0f, 2, 2, player, "PLAYER_ID");
		createText("PISTOL", {window_width_px - textHorizontalOffset, window_height_px - textVerticalOffset - 20 }, {255.0f, 255.0f, 255.0f}, 2.5f, 1.0f, 2, 2, player, "CURRENT_GUN");
		createText("20/20", { window_width_px - textHorizontalOffset, window_height_px - textVerticalOffset }, { 255.0f, 255.0f, 255.0f }, 2.5f, 1.0f, 2, 2, player, "AMMO_COUNT");
		createText("LIVES " + std::to_string(registry.players.get(player).lives), { window_width_px - textHorizontalOffset, window_height_px - textVerticalOffset + 20}, { 255.0f, 255.0f, 255.0f }, 2.5f, 1.0f, 2, 2, player, "HEALTH_COUNT");
	}

	//Create text for ammo counter and weapon
	createText("RED PLAYER", { textHorizontalOffset, window_height_px - textVerticalOffset - 40 }, { 255.0f, 0.0f, 0.0f }, 2.5f, 1.0f, horizontalAlignment, 2, player2, "PLAYER_ID");
	createText("PISTOL", { textHorizontalOffset, window_height_px - textVerticalOffset - 20 }, { 255.0f, 255.0f, 255.0f }, 2.5f, 1.0f, horizontalAlignment, 2, player2, "CURRENT_GUN");
	createText("20/20", { textHorizontalOffset, window_height_px - textVerticalOffset }, { 255.0f, 255.0f, 255.0f }, 2.5f, 1.0f, horizontalAlignment, 2, player2, "AMMO_COUNT");
	if (game_state_system->get_current_state() != 3) {
		createText("LIVES " + std::to_string(registry.players.get(player2).lives), { textHorizontalOffset, window_height_px - textVerticalOffset + 20 }, { 255.0f, 255.0f, 255.0f }, 2.5f, 1.0f, horizontalAlignment, 2, player2, "HEALTH_COUNT");
	}
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
				modifier.timer_ms = modifier.max_time_ms;
			}
			else {
				// if player does not have power up, modify players stats and add to powerup map
				playerStatModifier.powerUpStatModifiers[statModifier.name] = statModifier;

				StatUtil::apply_stat_modifier(player, statModifier);
			}

			if (game_state_system->get_current_state() == 3) {
				create_info_popup(statModifier.name);
			}

			random_drops_system->is_tutorial_intialized = false;
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
			Invincibility& invincibility = registry.invincibility.get(entity);
			Bullet& bullet = registry.bullets.get(entity_other);

			if (invincibility.has_TIMER) {
				continue;
			}
						
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

					printf("KNOCKBACK WITH PENALTY: %f\n", knockbackWithDropOff * hit_player.knockback_resistance);

					playerMotion.velocity.x += knockbackWithDropOff * (bullet_motion.velocity.x < 0 ? -1 : 1) * hit_player.knockback_resistance; 
				} else {
					float distanceBonus = distanceTravelled * bullet.distanceStrengthModifier;

					float knockbackWithBonus = bullet.knockback + distanceBonus;

					printf("KNOCKBACK WITH BONUS: %f\n", knockbackWithBonus * hit_player.knockback_resistance);

					playerMotion.velocity.x += knockbackWithBonus * (bullet_motion.velocity.x < 0 ? -1 : 1) * hit_player.knockback_resistance; 
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

				if (game_state_system->get_current_state() == 3) {
					create_info_popup(newGunComponent.name);
				}
			}
			random_drops_system->is_tutorial_intialized = false;
			registry.remove_all_components_of(entity_other);
		}
	}
	// Remove all collisions from player-mystery box
	registry.playerMysteryBoxCollisions.clear();
}

void WorldSystem::create_info_popup(std::string pickup_name) {
	// clear any existing text
	for (Entity entity : registry.deathTimers.entities) {
		registry.remove_all_components_of(entity);
	}

	auto placeholder_entity = Entity();


	std::string text1;
	std::string text2;
	std::string text3;

	if (pickup_name == "SUBMACHINE GUN") {
		text1 = "A light close quarters gun with weak knockback.";
		text2 = "However, it makes up for that deficiency with";
		text3 = "its fast fire rate and large magazine size.";
	}
	else if (pickup_name == "ASSAULT RIFLE") {
		text1 = "A heavier mid-range gun with medium knockback.";
		text2 = "It only has 20 rounds in its magazine, but the bullets";
		text3 = "travel faster and is effective at longer ranges.";
	}
	else if (pickup_name == "SNIPER RIFLE") {
		text1 = "A long range rifle that packs very high knockback.";
		text2 = "While it has a slow fire rate and very little ammo,";
		text3 = "the further your target is, the harder it hits.";
	}
	else if (pickup_name == "SHOTGUN") {
		text1 = "A powerful close quarters weapon, with extremely";
		text2 = "high knockback. However, its range is very";
		text3 = "limited and relys on being close to the target.";
	}
	else if (pickup_name == "Triple Jump") {
		text1 = "Gives the user an extra jump while in the air.";
	}
	else if (pickup_name == "Speed Boost") {
		text1 = "Boosts the player's movement speed.";
	}
	else if (pickup_name == "Super Jump") {
		text1 = "Boosts the player's jump height.";
	}
	
	// title
	auto popup_text_title = createText(pickup_name, { 1100, 70 }, { 255.0f, 255.0f, 255.0f }, 3.f, 1.0f, 2, 0, placeholder_entity, "PICKUP_INFO");
	auto popup_text_desc1 = createText(text1, { 1100, 100 }, { 255.0f, 255.0f, 255.0f }, 2.f, 0.7f, 2, 0, placeholder_entity, "PICKUP_INFO");
	auto popup_text_desc2 = createText(text2, { 1100, 125 }, { 255.0f, 255.0f, 255.0f }, 2.f, 0.7f, 2, 0, placeholder_entity, "PICKUP_INFO");
	auto popup_text_desc3 = createText(text3, { 1100, 150 }, { 255.0f, 255.0f, 255.0f }, 2.f, 0.7f, 2, 0, placeholder_entity, "PICKUP_INFO");

	float text_duration = 5000.0f; // in milliseconds
	registry.deathTimers.insert(popup_text_title, DeathTimer{ text_duration });
	registry.deathTimers.insert(popup_text_desc1, DeathTimer{ text_duration });
	registry.deathTimers.insert(popup_text_desc2, DeathTimer{ text_duration });
	registry.deathTimers.insert(popup_text_desc3, DeathTimer{ text_duration });
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
		while (registry.texts.entities.size() > 0)
	    	registry.remove_all_components_of(registry.texts.entities.back());
		game_state_system->change_game_state(0);
	}

	if (!paused) {
		if (game_state_system->get_current_state() == 2) {
			handle_player(key, action, player);
		}
		handle_player(key, action, player2);
		

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


