#pragma once

#include "common.hpp"
#include "game_state_system.hpp"
#include "render_system.hpp"

class MainMenuSystem
{
public:
    struct Button {
        vec3 color;
        vec2 pos;
        vec2 size;
        std::string text;
    };

    std::vector<Button> buttons;

    MainMenuSystem();

    void initialize_main_menu(RenderSystem* renderer_arg, GameStateSystem* game_state_system, GLFWwindow* window);
    void createMenuBackground(RenderSystem* renderer, const vec2& position, const vec2& size);
    void create_button(RenderSystem* renderer, const Button& button);
    bool is_point_inside_button(const Button& button, double x, double y);
    bool is_hovering_button(double x, double y);

	void on_key(int key, int action, int mod);
    void on_click();
    void on_mouse_move(vec2 mouse_position);

private:
    RenderSystem* renderer;
    GameStateSystem* game_state_system;
    GLFWwindow* window;
};