#pragma once

#include "input_system.hpp"

void InputSystem::init(RenderSystem* renderer_arg, GameStateSystem* game_state_system, GLFWwindow* window, WorldSystem* world_system, MainMenuSystem* main_menu_system) {
    this->renderer = renderer_arg;
    this->game_state_system = game_state_system;
    this->window = window;
    this->world_system = world_system;
    this->main_menu_system = main_menu_system;

    glfwSetWindowUserPointer(window, this);

    auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) {
        ((InputSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
        };
    auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
        ((InputSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 });
        };
    auto mouse_button_redirect = [](GLFWwindow* wnd, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            ((InputSystem*)glfwGetWindowUserPointer(wnd))->on_click();
        }
        };

    glfwSetKeyCallback(window, key_redirect);
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, mouse_button_redirect);
}


void InputSystem::on_key(int key, int, int action, int mod) {
    if (game_state_system->get_current_state() == 0) {
        main_menu_system->on_key(key, action, mod);
    }
    else if (!world_system->paused) {
        world_system->on_key(key, action, mod);
    }

}

void InputSystem::on_click() {
    if (game_state_system->get_current_state() == 0 || game_state_system->get_current_state() == 1) {
        main_menu_system->on_click();
    }
}

void InputSystem::on_mouse_move(vec2 mouse_position) {
    if (game_state_system->get_current_state() == 0 || game_state_system->get_current_state() == 1) {
        main_menu_system->on_mouse_move(mouse_position);
    }
}
