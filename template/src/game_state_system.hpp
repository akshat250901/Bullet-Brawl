#pragma once

// internal
#include "common.hpp"

class GameStateSystem 
{
public:
    // Game states
    enum GameState {
        MainMenu,
        GameWorld,
        Tutorial,
        Paused,
    };

    GameStateSystem();
    ~GameStateSystem();

    int get_current_state();
    void change_game_state(int newState);

    GLFWwindow* create_window();
    bool is_over()const;

	GLFWwindow* window;
    bool is_state_changed = false;
    bool is_quit = false;

private:
    int currentState;
     
};