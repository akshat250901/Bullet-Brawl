#include "camera_control_system.hpp"
#include "world_system.hpp"
#include <iostream>

CameraControlSystem::CameraControlSystem(GameStateSystem* gameStateSystem)
    : game_state_system(gameStateSystem) {
}

void CameraControlSystem::update_camera(float elapsed_ms) {
    if (game_state_system->get_current_state() == GameStateSystem::Winner) {
        int winnerId = game_state_system->get_winner();
        if (winnerId != -1) {
            focus_on_winner(elapsed_ms);
        }
    }
}

void CameraControlSystem::focus_on_winner(float elapsed_ms) {
    if (elapsedTime < focusDuration) {
        glm::vec2 winnerPosition = get_winner_position();

        // Smoothly interpolate camera's position and scale
        camera.position = lerp<glm::vec2>(camera.position, winnerPosition, lerpRate);
        camera.scale = lerp<float>(camera.scale, targetScale, lerpRate);
        std::cout << "Camera Position: " << camera.position.x << ", " << camera.position.y << " | Scale: " << camera.scale << std::endl;

        elapsedTime += elapsed_ms; // Increment elapsed time by the time since last frame
    }
    else {
        // Reset camera after focus duration
        camera.position = initialCameraPos;
        camera.scale = initialCameraScale;
        elapsedTime = 0.0f; // Reset the timer
    }

}

template<typename T>
T CameraControlSystem::lerp(const T& a, const T& b, float t) {
    return a + t * (b - a);
}

glm::vec2 CameraControlSystem::get_winner_position() {
    auto& player_container = registry.players;

    for (int i = 0; i < player_container.size(); i++) {
        Player& player_i = player_container.components[i];
        Entity entity_i = player_container.entities[i];

        if (player_i.lives != 0) {
            Motion& player_motion = registry.motions.get(entity_i);
            vec2 pos= player_motion.position;
            std::cout << "Player Position: " << pos.x << ", " << pos.y << std::endl;
            return player_motion.position;
        }

    }

    return { 0.0f,0.0f };
}


const CameraControlSystem::Camera& CameraControlSystem::get_camera() {
    return camera;
}
