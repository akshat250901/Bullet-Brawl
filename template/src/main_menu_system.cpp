#include "main_menu_system.hpp"
#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

MainMenuSystem::MainMenuSystem() {

};

void MainMenuSystem::initialize_main_menu(RenderSystem* renderer_arg, GameStateSystem* game_state_system, GLFWwindow* window) {
    this->game_state_system = game_state_system;
    this->window = window;
    this->renderer = renderer_arg;


    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int window_width = mode->width;
	int window_height = mode->height;

	if (game_state_system->get_current_state() == 0) {
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width / 2, window_height / 1.80f }, { window_width / 4, window_height / 16.5f }, "Start Game"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width / 2, window_height / 1.48f }, { window_width / 4, window_height / 16.5f }, "Tutorial"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width / 2, window_height / 1.28f }, { window_width / 4, window_height / 16.5f }, "End Game"});
		for (Button button: buttons) {
			create_button(renderer, button);
		}
	} else if (game_state_system->get_current_state() == 1) {
        buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width / 2, window_height / 2.7f }, { window_width / 3.4f, window_height / 16 }, "Island"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width / 2, window_height / 2 }, { window_width / 3.4f, window_height / 16 }, "Jungle"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width / 2, window_height / 1.6f }, { window_width / 3.4f, window_height / 16 }, "Space"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width / 2, window_height / 1.3333f }, { window_width / 3.4f, window_height / 16 }, "Temple"});
		buttons.push_back({{ 0.8f, 0.8f, 1.0f }, { window_width / 2, window_height / 1.14285f }, { window_width / 3.4f, window_height / 16 }, "back"});
		for (Button button: buttons) {
			create_button(renderer, button);
		}
	}

    createMenuBackground(renderer, { window_width_px / 2, window_height_px / 2 }, { window_width_px + 200, window_height_px });

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

void MainMenuSystem::on_key(int key, int action, int mod) {
     if (game_state_system->get_current_state() == 0)  {
         if (game_state_system && action == GLFW_RELEASE) {
             if (key == GLFW_KEY_SPACE) {
                 game_state_system->change_game_state(1);
             }
             else if (key == GLFW_KEY_TAB) {
                 game_state_system->change_game_state(3);
             }
             else if (key == GLFW_KEY_ESCAPE) {
                 game_state_system->is_quit = true;
             }
         }
     }    
}

void MainMenuSystem::on_click() {
    if (!buttons.empty() && game_state_system->get_current_state() != 2) {
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
    else if (game_state_system->get_current_state() == -1) // story
    {
        printf("we in cutscene\n");
    }
}

void MainMenuSystem::on_mouse_move(vec2 mouse_position) {
    if (game_state_system->get_current_state() != -1) {
        for (Button button : buttons) {
            if (is_point_inside_button(button, mouse_position.x, mouse_position.y)) {
                glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR));
                return;
            }
        }
        glfwSetCursor(window, nullptr);
    }
}
