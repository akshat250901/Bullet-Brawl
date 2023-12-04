#pragma once

#include "story_system.hpp"
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"


void StorySystem::init(RenderSystem* renderer, GameStateSystem* game_state_system, SoundSystem* sound_system) {
	this->renderer = renderer;
	this->game_state_system = game_state_system;
	this->sound_system = sound_system;
	
	// create frames of tutorial
	frames.push_back({ ".", TEXTURE_ASSET_ID::STORY_BLACK });
	frames.push_back({ "..", TEXTURE_ASSET_ID::STORY_BLACK });
	frames.push_back({ "...", TEXTURE_ASSET_ID::STORY_BLACK });
	frames.push_back({ "How could you...", TEXTURE_ASSET_ID::STORY_BLACK });
	frames.push_back({ "", TEXTURE_ASSET_ID::STORY_HOUSE2 });
	frames.push_back({ "How could you do such a thing..?", TEXTURE_ASSET_ID::STORY_HOUSE });
	frames.push_back({ "Why...?", TEXTURE_ASSET_ID::STORY_HOUSE });
	frames.push_back({ "...", TEXTURE_ASSET_ID::STORY_BLACK });
	frames.push_back({ "I will find you...", TEXTURE_ASSET_ID::STORY_PHOTO });
	frames.push_back({ "And I will never forgive you.", TEXTURE_ASSET_ID::STORY_BLACK });

	render_frame(current_frame);

	sound_system->play_burning_sound();
}

void StorySystem::render_frame(int index) {

	std::string text = frames.at(index).text;
	TEXTURE_ASSET_ID background_image = frames.at(index).background;

	std::tie(prev_frame, prev_text) = createStoryFrame(renderer, text, background_image, {window_width_px / 2, window_height_px / 2}, {window_width_px, window_height_px});
}

void StorySystem::on_click() {
	current_frame++;

	if (prev_frame) {
		registry.remove_all_components_of(prev_frame);
		registry.remove_all_components_of(prev_text);

	}

	if (game_state_system) {
		if (current_frame == frames.size()) {
			sound_system->stop_music();
			sound_system->play_bgm(); // stop playing story sound and play bgm
			game_state_system->change_game_state(0);
			game_state_system->is_state_changed = true;
		}
		else {
			render_frame(current_frame);
		}
	}
	else {
		printf("DOOMED");
	}

}