#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "game_state_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "main_menu_system.hpp"
#include "story_system.hpp"

class InputSystem

{
private:
    RenderSystem* renderer;
    GameStateSystem* game_state_system;
    GLFWwindow* window;

    WorldSystem* world_system;
    MainMenuSystem* main_menu_system;
    StorySystem* story_system;

public:
    void init(RenderSystem* renderer_arg, GameStateSystem* game_state_system, GLFWwindow* window, WorldSystem* world_system, MainMenuSystem* main_menu_system, StorySystem* story_system);
    
    // Input callback functions
    void on_key(int key, int, int action, int mod);
    void on_click();
    void on_mouse_move(vec2 mouse_position);

    InputSystem()
    {
    }
};