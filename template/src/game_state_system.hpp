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
    void change_game_state(int newState);

    GLFWwindow* create_window();
    bool is_over()const;

	GLFWwindow* window;
    bool is_state_changed = false;
    bool is_quit = false;

private:
    int currentState;

    // music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;
     
};