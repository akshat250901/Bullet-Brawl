#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BULLET, mesh_path("bullet.obj")),
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PROJECTILE, mesh_path("projectile.obj"))
		// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("red_health.png"),
			textures_path("green_health.png"),
			textures_path("rocket.png"),
			textures_path("player_spritesheet.png"),
			textures_path("powerup_spritesheet.png"),
			textures_path("background.png"),
			textures_path("middleground.png"),
			textures_path("foreground.png"),
			textures_path("menuBackground.png"),
			textures_path("levelSelectBackground.png"),
			textures_path("islandPlatform.png"),
			textures_path("islandPlatformTutorial.png"),
			textures_path("pistol.png"),
			textures_path("smg.png"),
			textures_path("ar.png"),
			textures_path("sniper.png"),
			textures_path("shotgun.png"),
			textures_path("shotgun_muzzle.png"),
			textures_path("weapon_box.png"),
			textures_path("JUNGLEMAP.png"),
			textures_path("SPACEMAP.png"),
			textures_path("TEMPLEMAP.png"),
			textures_path("TUTORIALMAP.png"),
			textures_path("popup_text/speedboost.png"),
			textures_path("popup_text/superjump.png"),
			textures_path("popup_text/triplejump.png"),
			textures_path("popup_text/smg.png"),
			textures_path("popup_text/ar.png"),
			textures_path("popup_text/sniper.png"),
			textures_path("popup_text/shotgun.png"),
			textures_path("popup_text/reload.png"),
			textures_path("red_arrow.png"),
			textures_path("green_arrow.png"),
			textures_path("green_player_won.png"),
			textures_path("red_player_won.png"),
			textures_path("story/black.png"),
			textures_path("story/house.png"),
			textures_path("story/house_no_overlay.png"),
			textures_path("story/photo.png"),
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("pebble"),
		shader_path("player"),
		shader_path("textured"),
		shader_path("water"),
		shader_path("background"),
		shader_path("animated"),
		shader_path("bullet"),
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();
	void drawAnimated(Entity entity, EFFECT_ASSET_ID asset_id);
	void drawText(int viewportWidth, int viewportHeight);

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;


	GLuint vao_rebind;
	bool initTextRender = false;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
