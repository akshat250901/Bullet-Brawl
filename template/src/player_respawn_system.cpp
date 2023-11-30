// internal
#include "player_respawn_system.hpp"
#include "world_init.hpp"
#include "create_gun_util.cpp"

const float KILL_LIMIT = 800.0f;
const float Y_HEIGHT_RESPAWN = -600.0f;

PlayerRespawnSystem::PlayerRespawnSystem(RenderSystem* renderSystem, GameStateSystem* gameStateSystem, SoundSystem* sound_system)
    : renderer(renderSystem), game_state_system(gameStateSystem), sound_system(sound_system) {
    rng = std::default_random_engine(std::random_device()());
}

void PlayerRespawnSystem::step()
{
    auto& player_container = registry.players;

    for (int i = 0; i < player_container.size(); i++) {
        Player& player_i = player_container.components[i];
        Entity entity_i = player_container.entities[i];

        Motion& player_motion = registry.motions.get(entity_i);
        PlayerStatModifier& player_stat_modifier = registry.playerStatModifiers.get(entity_i);

        Invincibility& invincibility = registry.invincibility.get(entity_i);

        // Check if players are out of window
        if (player_motion.position.y > window_height_px + abs(player_motion.scale.y / 2) +  KILL_LIMIT) {
            if (game_state_system->get_current_state() == 2) {

                auto health_container = registry.lives;
                for (int j = 0; j < health_container.components.size(); j++) {
                    Life health_entity = health_container.components[j];
                    if (health_entity.player == entity_i) {
                        registry.renderRequests.remove(health_container.entities[j]);
                        registry.lives.remove(health_container.entities[j]);
                        player_i.lives = player_i.lives - 1;
                        // Death animations
                        sound_system->play_fall_sound();
                        std::string player_text = player_i.color[1] == 1.f ? "GREEN" : "RED";
                        glm::vec3 result = player_i.color * vec3(255.0f, 255.0f, 255.0f);

                        for (int i = 0; i < registry.texts.size(); i++) {
                            Text& text_i = registry.texts.components[i];

                            if (text_i.tag == "PLAYER_FALL") {
                                registry.remove_all_components_of(registry.texts.entities[i]);
                                break;
                            }
                        }

                        Entity text = createText("-1 to " + player_text, {window_width_px / 2, 100 }, result, 4.0f, 1.0f, 1, 1, entity_i, "PLAYER_FALL", 1000.0f);

                        if (player_i.lives == 0) {
                            if (!registry.deathTimers.has(entity_i)) {
                                registry.deathTimers.emplace(entity_i);
                            }

                            while (registry.texts.entities.size() > 0)
                            {
                                registry.remove_all_components_of(registry.texts.entities.back());
                            }
                            if (player_i.color == glm::vec3{ 1.f, 0.f, 0.f }) {
                                game_state_system->set_winner(2);
                            }
                            else {
                                game_state_system->set_winner(1);
                            }
                        }
                        break;
                    }
                }
                if (game_state_system->get_winner() == -1)
                    {
                        auto& platformsContainer = registry.platforms;

                        int furthestLeft = std::numeric_limits<int>::max();
                        int furthestRight = std::numeric_limits<int>::min();

                        for (int i = 0; i < platformsContainer.size(); i++) {
                            Entity entity_i = platformsContainer.entities[i];
                            Motion& platform_motion = registry.motions.get(entity_i);

                            float rightEdge = platform_motion.position.x + platform_motion.scale.x / 2;
                            float leftEdge = platform_motion.position.x - platform_motion.scale.x / 2;

                            if (leftEdge < furthestLeft) {
                                furthestLeft = leftEdge;
                            }

                            if (rightEdge > furthestRight) {
                                furthestRight = rightEdge;
                            }
                        }

                        // Player death logic
                        int xCoord = uniform_dist_int(rng, decltype(uniform_dist_int)::param_type{furthestLeft, furthestRight});

                        player_motion.position = vec2(xCoord, Y_HEIGHT_RESPAWN);
                        player_motion.velocity = vec2(0, 0);

                        invincibility.has_TIMER = true;
                        invincibility.timer_ms = invincibility.max_time_ms;
                        invincibility.player_original_color = player_i.color;
                        player_i.color = invincibility.invincibility_color;

                        CreateGunUtil::givePlayerStartingPistol(renderer, entity_i, true);
                    }
            }
            else if (game_state_system->get_current_state() == 3) {
                if (i == 0) {
                    player_motion.position = vec2(300, 200);
                    player_motion.velocity = vec2(0, 0);
                }
                if (i == 1) {
                    player_motion.position = vec2(700, 200);
                    player_motion.velocity = vec2(0, 0);
                }
                
            }

            // Set timer to 0 for all power ups to stats are reset
            for (auto& kv : player_stat_modifier.powerUpStatModifiers) {
                kv.second.timer_ms = 0;
            }

        }
    }
}

