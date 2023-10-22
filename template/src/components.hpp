#pragma once
#include "common.hpp"
#include <set>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{
	// If player is colliding with a platform, they are grounded
	bool is_grounded = false;
	// Direction player is facing for sprite (default facing right)
	bool facing_right = true;
	// Max number of jumps allowed in the air
	int max_jumps = 1;
	// Number of jumps available after the player is no longer grounded
	int jump_remaining = 1;
	// Jump force
	float jump_force = 400.0f;
	// Running force
	float running_force = 400.f;
	// Running speed
	float speed = 200.f;
	// True if player is running left
	bool is_running_left = false; 
	// True if player is running right
	bool is_running_right = false;
	// True if the player is shooting
	bool is_shooting = false;
	// recoil force
	float recoil_force = 2000.f;
};

// Player stat modifier struct for use in power ups and gun pick ups
struct StatModifier
{
	std::string name = "";

	bool hasTimer = false;
	float timer_ms = 0;

	int extra_jumps = 0;
	float jump_force_modifier = 1;
	float running_force_modifier = 1;
	float max_speed_modifier = 1;
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

// Turtles have a hard shell
struct HardShell
{

};

// Fish and Salmon have a soft shell
struct SoftShell
{

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

};

// Background Parallax
struct ParallaxBackground
{
	float scrollingSpeeed = 0.1f;
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	float angle = 0.f;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	Collision(Entity& other_entity) { this->other_entity = other_entity; };
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
	float timer_ms = 3000.f;
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
	FISH = 0,
	TURTLE = FISH + 1,
	PLAYER = TURTLE + 1,
	PLAYER2 = PLAYER + 1,
	BACKGROUND = PLAYER2 + 1,
	MIDDLEGROUND = BACKGROUND + 1,
	FOREGROUND = MIDDLEGROUND + 1,
	TEXTURE_COUNT = FOREGROUND + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PEBBLE = COLOURED + 1,
	PLAYER = PEBBLE + 1,
	TEXTURED = PLAYER + 1,
	WATER = TEXTURED + 1,
	BACKGROUND = WATER + 1,
	EFFECT_COUNT = WATER + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SALMON = 0,
	SPRITE = SALMON + 1,
	PEBBLE = SPRITE + 1,
	DEBUG_LINE = PEBBLE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	SQUARE = SCREEN_TRIANGLE + 1,
	GEOMETRY_COUNT = SQUARE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

