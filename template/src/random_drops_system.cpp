// internal
#include "random_drops_system.hpp"
#include "world_init.hpp"

// Drop timing
const size_t MAX_POWERUPS = 5;
const float POWERUP_DELAY_MS = 10000;
const float POWERUP_SIZE = 50.f;

RandomDropsSystem::RandomDropsSystem(RenderSystem* renderSystem)
    : next_powerup_spawn(POWERUP_DELAY_MS)
    , renderer(renderSystem) {
        rng = std::default_random_engine(std::random_device()());
}

void RandomDropsSystem::step(float elapsed_ms_since_last_update)
{
    // Power up spawn
    next_powerup_spawn -= elapsed_ms_since_last_update;

	if (registry.powerUps.components.size() >= MAX_POWERUPS) {
        next_powerup_spawn = POWERUP_DELAY_MS;
    }

    if (next_powerup_spawn < 0.f) {
        // Reset timer
        next_powerup_spawn = (POWERUP_DELAY_MS / 2) + uniform_dist(rng) * (POWERUP_DELAY_MS / 2);

        // Generate xPos and yPos to place (can refactor into private method up to comment HERE)
        std::vector<Entity> platforms = registry.platforms.entities;

        int maxPlatformIndex = platforms.size() - 1;
        int platformNum = uniform_dist_int(rng, decltype(uniform_dist_int)::param_type{0, maxPlatformIndex});

        Entity randomPlatform = platforms.at(platformNum);

        Motion& platformMotion = registry.motions.get(randomPlatform);

        // Get random x coordinate along the platform
        int xCoord = uniform_dist_int(rng, decltype(uniform_dist_int)::param_type{(int) platformMotion.position.x - (int) (platformMotion.scale.x / 2) + (int) POWERUP_SIZE, (int) platformMotion.position.x + (int) (platformMotion.scale.x / 2) - (int) POWERUP_SIZE});
        int yCoord = (int) (platformMotion.position.y) - (int) (platformMotion.scale.y / 2) - (int) (POWERUP_SIZE / 2);

        vec2 powerUpPos = {xCoord, yCoord};
        // HERE

        // Get random power up stat modifier
        int numPowerUps = powerUpsNames.size() - 1;
        int powerUpNum = uniform_dist_int(rng, decltype(uniform_dist_int)::param_type{0, numPowerUps});
        std::string powerUpName = powerUpsNames.at(powerUpNum);

        auto color = powerUpsNamesToColor[powerUpName];
        auto statModifier = powerUpsNamesToStatModifier[powerUpName];

        Entity entity = createPowerup(renderer, powerUpPos, {POWERUP_SIZE, POWERUP_SIZE}, color);

        PowerUp& powerUp = registry.powerUps.emplace(entity);
        powerUp.statModifier = statModifier;
    }
}

void RandomDropsSystem::handleInterpolation(float elapsed_ms) {
            for (Entity entity : registry.powerUps.entities) {
                if (registry.interpolation.has(entity) && registry.motions.has(entity))
                {
                    Interpolation& interpolation = registry.interpolation.get(entity);
                    Motion& motion = registry.motions.get(entity);

                    interpolation.currentTime += elapsed_ms;
                    float frequency = 0.002f;  // Adjust this to change the speed
                    float t = std::sin(interpolation.currentTime * frequency);

                    // Map t from [-1, 1] to [0, 1]
                    t = (t + 1.0f) / 2.0f;

                    motion.position.x = interpolation.startPosition.x + t * (interpolation.endPosition.x - interpolation.startPosition.x);
                    motion.position.y = interpolation.startPosition.y + t * (interpolation.endPosition.y - interpolation.startPosition.y);
                }
            
            }
}

