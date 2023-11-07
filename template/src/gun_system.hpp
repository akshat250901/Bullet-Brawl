#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "sound_system.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class GunSystem
{
private:
    RenderSystem* renderer;
	SoundSystem* sound_system;

	void animateRecoil(Gun& gun, Motion& gun_motion, const Player& player_component);

public:
	void step(float elapsed_ms);

	GunSystem(RenderSystem* renderer, SoundSystem* sound_system);
};