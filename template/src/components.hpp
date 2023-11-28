#pragma once
#include "common.hpp"
#include <set>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <map>


struct Life
{
	Entity player; 
	Life(Entity& player) { this->player = player; };
};

struct Keybinds
{
	int up = 0;
	int down = 0;
	int left = 0;
	int right = 0;

	int bullet = 0;
	int projectile = 0;
};

// Player component
struct Player
{
	// Player color
	vec3 color = { 1, 1, 1};
	// Player Keybinds
	Keybinds keybinds;
	// If player is colliding with a platmorm, they are grounded
	bool is_grounded = false;
	// Direction player is facing for sprite (default facing right)
	bool facing_right = true;
	// Max number of jumps allowed in the air
	int max_jumps = 1;
	// Number of jumps available after the player is no longer grounded
	int jump_remaining = 1;
	// Jump force
	float jump_force = 430.0f;
	// Running force
	float running_force = 850.f;
	// Running speed
	float speed = 200.f;
	// Knockback resistance
	float knockback_resistance = 1.0f;
	// True if player is running left
	bool is_running_left = false; 
	// True if player is running right
	bool is_running_right = false;
	//lives
	int lives;
};

struct PopupIndicator
{
	std::string type = "Default pistol";
	Entity player;
	float timer = 1000.f; // in ms
};

struct Controller
{
	bool leftKey = false;
	bool rightKey = false;
	bool upKey = false;
	bool downKey = false;

	bool fireKey = false;
	bool abilityKey = false;

	float upKey_delay_timer_ms = 300.0f;
};

// Player invincibility component
struct Invincibility 
{
	bool has_TIMER = false;
	float timer_ms = 0.0f;
	float max_time_ms = 4000.0f;
	vec3 player_original_color;
	vec3 invincibility_color = vec3(0.f, 0.f, 0.f);
};

// Player stat modifier struct for use in power ups and gun pick ups
struct StatModifier
{
	std::string name = "";

	bool hasTimer = false;
	float timer_ms = 0.0f;
	float max_time_ms = 0.0f;

	int extra_jumps = 0;
	float jump_force_modifier = 1.0f;
	float running_force_modifier = 1.0f;
	float max_speed_modifier = 1.0f;
	float knockback_resistance_modifier = 0.0f;
};

// Stat Modifier component
struct PlayerStatModifier
{
	std::unordered_map<std::string, StatModifier> powerUpStatModifiers;
};

// Power Up
struct PowerUp
{
	StatModifier statModifier;
};

// Struct for gun and default values are the starting pistol
struct Gun 
{
	std::string name = "PISTOL";

	Entity gunOwner;

	StatModifier statModifier;

	float fireRateMs = 300.0f;
	float fireRateTimerMs = 0.0f;

	float bulletVelocity = 1000.0f;
	float knockback = 1000.0f; // This is added to players x velocity
	bool hasNormalDropOff = true;
	float distanceStrengthModifier = 0.8;

	float recoil = 30.0f;
	float recoilAnimationModifier = 0.5f; // Faster animation as this value gets closer to 1

	bool isHitScan = false;
	float hitScanRange = 0.0f; //number of pixels

	bool currentlyReloading = false;
	float reloadMs = 1000.0f;
	float reloadTimerMs = reloadMs;

	bool hasInfiniteAmmo = false;
	int magazineSize = 10; // Max capacity of magazine
	int magazineAmmo = 10; // Current ammo in magazine
	int reserveAmmo = magazineSize * 2;

	vec2 gunSize = {30.0f, 30.0f};
};

struct MuzzleFlash {
	float timerMs = 100.0f;
	bool facing_right;
};

struct NonInteractable {

};

struct GunMysteryBox {
	Gun randomGun;
};

struct Interpolation
{
	vec2 startPosition;
	vec2 endPosition;
	float currentTime = 0;  // Current time elapsed since the start of the movement
};

// Component for sprite sheet
struct AnimatedSprite {
	float sprite_height = 0.25;
	float sprite_width = 0.125;

	float animation_speed_ms = 100;
	float ms_since_last_update = 0;

	int animation_type = 0;  // y axis - relates to player movement_state
	int animation_frame = 0; // x axis - frame of animation

	std::map<int, int> frame_count_per_type = {};
};

// Platforms
struct Platform
{
	bool collider_active_player1 = true;
	bool collider_active_player2 = true;
};

// Background
struct Background
{
	
};

// Horizontal Friction
struct Friction
{

};

// Gravity
struct Gravity
{
	// This value is how strong the gravity is
	float force = 800.0f;
};

// Bullet
struct Bullet
{
	float originalXPosition = 0.0f;

	bool isHitscan = false;
	bool hasNormalDropOff = true;
	float distanceStrengthModifier = 1; // if normal drop off, lower value less penalty, if non normal drop off then lower value more penalty

	float knockback = 0.0f;
	
	Entity shooter; // owner of bullet
};

// Background Parallax
struct ParallaxBackground
{
	float scrollingSpeedBack = 0.0f;
	float scrollingSpeedMiddle = 0.0f;
	float scrollingSpeedFront= 0.0f;
	float scrollingSpeed = 0.0f;
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	float angle = 0.f;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
};

// Component to store text rendering data

struct Text {
	std::string string = "";
	vec2 position = { 0.f, 0.f };
	vec3 color = { 0.f, 0.f, 0.f };
	float scale = 1.0f;
	float opacity = 1.0f;

	// 0 is LEFT OR TOP
	// 1 is CENTER
	// 2 is RIGHT OR BOTTOM

	int horizontalAlignment = 1;
	int verticalAlignment = 1;

	Entity owner;
	std::string tag;
	float timer_ms;
};


// Stucture to store collision information
struct PlayerPlatformCollision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	PlayerPlatformCollision(Entity& other_entity) { this->other_entity = other_entity; };
};

struct PlayerCollectibleCollisions
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	PlayerCollectibleCollisions(Entity& other_entity) { this->other_entity = other_entity; };
};

// Stucture to store collision information
struct PlayerBulletCollision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	PlayerBulletCollision(Entity& other_entity) { this->other_entity = other_entity; };
};

// Stucture to store collision information
struct PlayerMysteryBoxCollision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	PlayerMysteryBoxCollision(Entity& other_entity) { this->other_entity = other_entity; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float screen_darken_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float timer_ms = 1000.f;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	RED_HEALTH = 0,
	GREEN_HEALTH = RED_HEALTH + 1,
	PLAYER_SPRITESHEET = GREEN_HEALTH + 1,
	POWERUP_SPRITESHEET = PLAYER_SPRITESHEET + 1,
	BACKGROUND = POWERUP_SPRITESHEET + 1,
	MIDDLEGROUND = BACKGROUND + 1,
	FOREGROUND = MIDDLEGROUND + 1,
	MENUBACKGROUND = FOREGROUND + 1,
	LEVELSELECTBACKGROUND = MENUBACKGROUND + 1,
	PLATFORM = LEVELSELECTBACKGROUND + 1,
	TUTORIALPLATFORM = PLATFORM + 1,
	PISTOL = TUTORIALPLATFORM + 1,
	SMG = PISTOL + 1,
	AR = SMG + 1,
	SNIPER = AR + 1,
	SHOTGUN = SNIPER + 1,
	SHOTGUN_MUZZLE = SHOTGUN + 1,
	WEAPON_BOX = SHOTGUN_MUZZLE + 1,
	JUNGLEMAP = WEAPON_BOX + 1,
	SPACEMAP = JUNGLEMAP + 1,
	TEMPLEMAP = SPACEMAP + 1,
	TUTORIALMAP = TEMPLEMAP + 1,
	SPEEDBOOST = TUTORIALMAP + 1,
	SUPERJUMP = SPEEDBOOST + 1,
	TRIPLEJUMP = SUPERJUMP + 1,
	SMG_PICKUP = TRIPLEJUMP + 1,
	AR_PICKUP = SMG_PICKUP + 1,
	SNIPER_PICKUP = AR_PICKUP + 1,
	SHOTGUN_PICKUP = SNIPER_PICKUP + 1,
	RELOAD_TEXT = SHOTGUN_PICKUP + 1,
	RED_ARROW = RELOAD_TEXT + 1,
	GREEN_ARROW = RED_ARROW + 1,
	GREEN_PLAYER_WON = GREEN_ARROW + 1,
	RED_PLAYER_WON = GREEN_PLAYER_WON + 1,
	TEXTURE_COUNT = RED_PLAYER_WON + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PEBBLE = COLOURED + 1,
	PLAYER = PEBBLE + 1,
	TEXTURED = PLAYER + 1,
	WATER = TEXTURED + 1,
	BACKGROUND = WATER + 1,
	ANIMATED = BACKGROUND + 1,
	EFFECT_COUNT = ANIMATED + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	BULLET = 0,
	SPRITE = BULLET + 1,
	PEBBLE = SPRITE + 1,
	DEBUG_LINE = PEBBLE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	SQUARE = SCREEN_TRIANGLE + 1,
	ANIMATED_SPRITE = SQUARE + 1,
	PROJECTILE = ANIMATED_SPRITE + 1,
	GEOMETRY_COUNT = PROJECTILE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};


// Out of bounds arrow
struct OutOfBoundsArrow {
	Entity entity_to_track;
	TEXTURE_ASSET_ID textureId;
};

