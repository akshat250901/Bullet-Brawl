#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "game_state_system.hpp"


// A camera control system which focuses on the winning player
class CameraControlSystem
{
public:
    struct Camera {
        glm::vec2 position;
        float scale;

        Camera() : position(glm::vec2(0.0f, 0.0f)), scale(1.0f) {}

    };

    const Camera& get_camera();

    Camera camera;

    CameraControlSystem(GameStateSystem* gameStateSystem);
    void update_camera(float elapsed_ms);

private:

    GameStateSystem* game_state_system;
    void focus_on_winner(float elapsed_ms);
    glm::vec2 get_winner_position();

    template<typename T>
    T lerp(const T& a, const T& b, float t);

    float elapsedTime = 0.0f;
    const float focusDuration = 3000.0f; // Duration in seconds to focus on the winner
    const float targetScale = 2.0f;   // Target zoom level for focusing
    const float lerpRate = 0.1f;      // Rate of interpolation for smoothness

    glm::vec2 initialCameraPos = glm::vec2(0.0f, 0.0f);
    float initialCameraScale = 1.0f;
};