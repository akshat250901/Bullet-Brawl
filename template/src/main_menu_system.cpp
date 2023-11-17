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

    // createMenuBackground(renderer, { window_width_px / 2, window_height_px / 2 }, { window_width_px + 200, window_height_px });

    auto key_redirect = [](GLFWwindow* wnd, int key, int scancode, int action, int mods) {
        ((MainMenuSystem*)glfwGetWindowUserPointer(wnd))->on_key(key, scancode, action, mods);
    };

    auto mouse_button_redirect = [](GLFWwindow* wnd, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            ((MainMenuSystem*)glfwGetWindowUserPointer(wnd))->on_click();
        }
    };

    glfwSetKeyCallback(window, key_redirect);
    glfwSetMouseButtonCallback(window, mouse_button_redirect);

	if (game_state_system->get_current_state() == 0) {
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width_px / 2, 450 }, { 300, 70 }, "Start Game"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width_px / 2, 550 }, { 300, 70 }, "Tutorial"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width_px / 2, 650 }, { 300, 70 }, "End Game"});
		for (Button button: buttons) {
			create_button(renderer, button);
		}
	} else if (game_state_system->get_current_state() == 1) {
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width_px / 2, 400 }, { 300, 50 }, "Jungle"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width_px / 2, 500 }, { 300, 50 }, "Space"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width_px / 2, 600 }, { 300, 50 }, "Temple"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width_px / 2, 700 }, { 300, 50 }, "back"});
		for (Button button: buttons) {
			create_button(renderer, button);
		}
	}

    createMenuBackground(renderer, { window_width_px / 2, window_height_px / 2 }, { window_width_px + 200, window_height_px });

	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((MainMenuSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
}

void MainMenuSystem::create_button(RenderSystem* renderer, const Button& button) {
    vec3 color = button.color;
    vec2 position = button.pos;
    vec2 size = button.size;
    
    auto entity = Entity();

    // Store a reference to the potentially re-used mesh object
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
    registry.meshPtrs.emplace(entity, &mesh);

    // Initialize the position, scale, and physics components
    auto& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = { 0.f, 0.f };
    motion.position = position;

    // Setting initial values, scale is negative to make it face the opposite way
    motion.scale = size;
    registry.buttons.emplace(entity);
    registry.colors.insert(entity, color);
    registry.renderRequests.insert(
        entity,
        { TEXTURE_ASSET_ID::TEXTURE_COUNT,
          EFFECT_ASSET_ID::COLOURED,
          GEOMETRY_BUFFER_ID::SQUARE });
}

void MainMenuSystem::createMenuBackground(RenderSystem* renderer, const vec2& position, const vec2& size) {
    // Reserve an entity
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
	if (game_state_system->get_current_state() == 0) {
		registry.renderRequests.insert(
        entity,
        { TEXTURE_ASSET_ID::MENUBACKGROUND,
          EFFECT_ASSET_ID::BACKGROUND,
          GEOMETRY_BUFFER_ID::SPRITE });
	}
	if (game_state_system->get_current_state() == 1) {
		registry.renderRequests.insert(
        entity,
        { TEXTURE_ASSET_ID::LEVELSELECTBACKGROUND,
          EFFECT_ASSET_ID::BACKGROUND,
          GEOMETRY_BUFFER_ID::SPRITE });
	}
}

bool MainMenuSystem::is_point_inside_button(const Button& button, double x, double y) {
    return ((x >= button.pos.x - button.size.x / 2) && (x <= (button.pos.x + button.size.x / 2)) &&
            (y >= button.pos.y - button.size.y / 2) && (y <= (button.pos.y + button.size.y / 2)));
}

bool MainMenuSystem::is_hovering_button(double x, double y) {
	for (Button button : buttons) {
		if (is_point_inside_button(button, x, y)) {
			return true;
		}
	}
	return false;
}

void MainMenuSystem::on_key(int key, int, int action, int mod) {
    // if (game_state_system->get_current_state() == 0)  {
    //     if (game_state_system && action == GLFW_RELEASE) {
    //         if (key == GLFW_KEY_SPACE) {
    //             game_state_system->change_game_state(1);
    //         }
    //         else if (key == GLFW_KEY_TAB) {
    //             game_state_system->change_game_state(3);
    //         }
    //         else if (key == GLFW_KEY_ESCAPE) {
    //             game_state_system->is_quit = true;
    //         }
    //     }
    // }
    
}

void MainMenuSystem::on_click() {
    if (!buttons.empty()) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        if (game_state_system->get_current_state() == 0) {
			if (is_point_inside_button(buttons[0], x, y)) {
				game_state_system->change_game_state(1);
				buttons.clear();
				glfwSetCursor(window, nullptr);
			}

			else if (is_point_inside_button(buttons[1], x, y)) {
				game_state_system->change_game_state(3);
				glfwSetCursor(window, nullptr);
			}

			else if (is_point_inside_button(buttons[2], x, y)) {
				game_state_system->is_quit = true;
				glfwSetCursor(window, nullptr);
			}	
		} else if (game_state_system->get_current_state() == 1) {
			int i = 0;
			for (Button button : buttons) {
				i++;
				if (button.text == "back") {
					game_state_system->change_game_state(0);
                    buttons.clear();
				} else {
					if (is_point_inside_button(button, x, y)) {
						game_state_system->change_game_state(2);
						game_state_system->change_level(i);
						buttons.clear();
						glfwSetCursor(window, nullptr);
                        break;
					}
				}
			}
		}
    }
}

void MainMenuSystem::on_mouse_move(vec2 mouse_position) {
	for (Button button : buttons) {
		if (is_point_inside_button(button, mouse_position.x, mouse_position.y)) {
			glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR));
			return;
		}
	}
	glfwSetCursor(window, nullptr);
}
