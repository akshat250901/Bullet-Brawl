#include "main_menu_system.hpp"
#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

MainMenuSystem::MainMenuSystem() {

};

void MainMenuSystem::initialize_main_menu(RenderSystem* renderer_arg, GameStateSystem* game_state_system, GLFWwindow* window) {
    this->game_state_system = game_state_system;
    this->window = window;
	this->renderer = renderer_arg;

	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((MainMenuSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	// DEBUG CRASHES CAUSED BY MOUSE CLICKS
	// auto mouse_button_redirect = [](GLFWwindow* wnd, int button, int action, int mods) {
    // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    //     ((MainMenuSystem*)glfwGetWindowUserPointer(wnd))->on_click();
    // }};
	glfwSetKeyCallback(window, key_redirect);
	// glfwSetMouseButtonCallback(window, mouse_button_redirect);

	createMenuBackground(renderer, { window_width_px / 2, window_height_px / 2 }, { window_width_px + 200, window_height_px });

    // buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { 200, 200 }, { 200, 50 }, "Button 1"});
    // buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { 200, 400 }, { 200, 50 }, "Button 2"});
    // buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { 200, 600 }, { 200, 50 }, "Button 3"});

	// create_button(renderer, buttons[0]);
	// create_button(renderer, buttons[1]);
	// create_button(renderer, buttons[2]);
};

// void MainMenuSystem::create_button(RenderSystem* renderer, Button button) {
// 	vec3 color = button.color;
// 	vec2 position = button.pos;
// 	vec2 size = button.size;
	
// 	// Reserve en entity
// 	auto entity = Entity();

// 	// Store a reference to the potentially re-used mesh object
// 	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
// 	registry.meshPtrs.emplace(entity, &mesh);

// 	// Initialize the position, scale, and physics components
// 	auto& motion = registry.motions.emplace(entity);
// 	motion.angle = 0.f;
// 	motion.velocity = { 0.f, 0.f };
// 	motion.position = position;

// 	// Setting initial values, scale is negative to make it face the opposite way
// 	motion.scale = size;
// 	registry.buttons.emplace(entity);
// 	registry.colors.insert(entity, color);
// 	registry.renderRequests.insert(
// 		entity,
// 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
// 			EFFECT_ASSET_ID::COLOURED,
// 			GEOMETRY_BUFFER_ID::SQUARE });
// };

void MainMenuSystem:: createMenuBackground(RenderSystem* renderer, vec2 position, vec2 size)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = size;

	// Add the Parallax component for the back layer, which might move the slowest.
	ParallaxBackground& parallax = registry.parallaxes.emplace(entity);
	parallax.scrollingSpeedBack = 0.0f; // Adjust this value as needed.
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MENUBACKGROUND,
			EFFECT_ASSET_ID::BACKGROUND,
			GEOMETRY_BUFFER_ID::SPRITE });
};

// check if a point is inside a button's bounds
bool MainMenuSystem::is_point_inside_button(const MainMenuSystem::Button& button, double x, double y) {
    return (x >= button.pos.x && x <= (button.pos.x + button.size.x) &&
            y >= button.pos.y && y <= (button.pos.y + button.size.y));
};


void MainMenuSystem::on_key(int key, int, int action, int mod) {
    if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_SPACE) {
			game_state_system->change_game_state(1);
		}
		if (key == GLFW_KEY_TAB) {
			// TODO: Add tutorial level
			game_state_system->change_game_state(2);
		}
		if (key == GLFW_KEY_ESCAPE) {
			game_state_system->is_quit = true;
		}
    }
};


void MainMenuSystem::on_click() {
	// (vec2)mouse_position; // dummy to avoid compiler warning

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	// DEBUG CRASHES CAUSED BY MOUSE CLICKS

	// if (is_point_inside_button(buttons[0], x, y)) {
	// 	game_state_system->change_game_state(1);
	// }
	
	// if (is_point_inside_button(buttons[1], x, y)) {
		
	// }

	// if (is_point_inside_button(buttons[2], x, y)) {
	// 	game_state_system->is_quit = true;
	// }

};
