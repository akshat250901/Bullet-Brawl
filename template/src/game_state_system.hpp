#pragma once

// internal
#include "common.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

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
    int get_current_level();

    void change_game_state(int newState);
    void change_level(int newLevel);

    GLFWwindow* create_window();
    bool is_over()const;

	GLFWwindow* window;
    bool is_state_changed = false;
    bool is_quit = false;

private:
    // States are 0 for main menu, 1 for tutorial
    int currentState;
    // Levels are 1 for jungle, 2 for space, 3 for temple (default is 0 when in main menu)
    int currentLevel = 0;

    // music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;
     
};