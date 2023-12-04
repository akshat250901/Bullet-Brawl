#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "game_state_system.hpp"
#include "sound_system.hpp"

// System for story elements
class StorySystem
{
private:
	RenderSystem* renderer;
	GameStateSystem* game_state_system;
	SoundSystem* sound_system;

	Entity prev_frame;
	Entity prev_text;

	int current_frame = 0;

	std::vector<StoryFrame> frames;


public:
	void init(RenderSystem* renderer, GameStateSystem* game_state_system, SoundSystem* sound_system);
	void on_click();
	void render_frame(int index);
	StorySystem()
	{
	}
};