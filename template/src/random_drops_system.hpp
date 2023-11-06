#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

#include <random>
#include <unordered_map>
#include <string>

// A simple physics system that moves rigid bodies and checks for collision
class RandomDropsSystem
{
public:
	void step(float elapsed_ms);

    void handleInterpolation(float elapsed_ms);

	RandomDropsSystem(RenderSystem* renderer);

private:

    // Game state
    RenderSystem* renderer;
    float next_powerup_spawn;
    float next_mystery_box_spawn;

    // C++ random number generator
	std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;
	std::uniform_int_distribution<int> uniform_dist_int; // number between 0..1

    vec2 getRandomPositionOnPlatform();

    // Define a list of power-ups
    std::vector<std::string> powerUpsNames = {
        "Triple Jump", "Speed Boost", "Super Jump"
    };

    std::unordered_map<std::string, vec3> powerUpsNamesToColor = {
        {"Triple Jump", {1.0f, 0.5f, 0.5f}},
        {"Speed Boost", {0.5f, 1.0f, 0.5f}},
        {"Super Jump", {0.5f, 0.5f, 1.0f}}
    };

    std::unordered_map<std::string, StatModifier> powerUpsNamesToStatModifier = {
        {"Triple Jump", StatModifier{
                            "Triple Jump",
                            true,              
                            5000,             
                            1,                 
                            1.0f,              
                            1.0f,              
                            1.0f               
                        }},
        {"Speed Boost", StatModifier{
                            "Speed Boost",
                            true,              
                            5000,              
                            0,                 
                            1.0f,              
                            1.5f,              
                            1.5f               
                        }},
        {"Super Jump", StatModifier{
                            "Super Jump", 
                            true,              
                            5000,              
                            0,                 
                            1.3f,              
                            1.0f,              
                            1.0f               
                        }}
    };


    std::vector<Gun> guns;


};