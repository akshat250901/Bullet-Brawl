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

        
        float leftBoundary = camera.position.x;
        float rightBoundary = camera.position.x + window_width_px / camera.scale;
        float topBoundary = camera.position.y;
        float bottomBoundary = camera.position.y + window_height_px / camera.scale;

       
        float horizontalBuffer = window_width_px / (4 * camera.scale);
        float verticalBuffer = window_height_px / (4 * camera.scale);

        
        if (winnerPosition.x < leftBoundary + horizontalBuffer) {
            camera.position.x = winnerPosition.x - horizontalBuffer;
        }
        else if (winnerPosition.x > rightBoundary - horizontalBuffer) {
            camera.position.x = winnerPosition.x - (window_width_px / camera.scale) + horizontalBuffer;
        }

        if (winnerPosition.y < topBoundary + verticalBuffer) {
            camera.position.y = winnerPosition.y - verticalBuffer;
        }
        else if (winnerPosition.y > bottomBoundary - verticalBuffer) {
            camera.position.y = winnerPosition.y - (window_height_px / camera.scale) + verticalBuffer;
        }

        // Interpolate camera position and scale
        glm::vec2 targetPosition = { std::max(camera.position.x, leftBoundary), std::max(camera.position.y, topBoundary) };
        //glm::vec2 centeredCameraPosition = winnerPosition - glm::vec2(window_width_px / (2 * camera.scale), window_height_px / (2 * camera.scale));
        camera.position = lerp(camera.position, targetPosition, lerpRate);
        camera.scale = lerp<float>(camera.scale, targetScale, lerpRate);
        camera.position.x = std::min(camera.position.x, rightBoundary);
        camera.position.y = std::max(camera.position.y, topBoundary);
        camera.position.y = std::min(camera.position.y, bottomBoundary);
        elapsedTime += elapsed_ms; // Increment elapsed time
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
            return player_motion.position;
        }

    }
    return {0.0f,0.0f };
}


const CameraControlSystem::Camera& CameraControlSystem::get_camera() {
    return camera;
}

void CameraControlSystem::reset_camera() {
    camera.position = initialCameraPos;
    camera.scale = initialCameraScale;
}