// internal
#include "random_drops_system.hpp"
#include "world_init.hpp"

// Drop timing
const size_t MAX_POWERUPS = 5;
const float POWERUP_DELAY_MS = 15000.0f;
const float TUTORIAL_POWERUP_DELAY_MS = 2000.f;
const float POWERUP_SIZE = 50.f;

const size_t MAX_MYSTERY_BOX = 1;
const float MYSTERY_BOX_DELAY_MS = 12000.0f;
const float MYSTERY_BOX_SIZE = 40.0f;

RandomDropsSystem::RandomDropsSystem(RenderSystem* renderSystem)
    : next_powerup_spawn(POWERUP_DELAY_MS)
    , next_mystery_box_spawn(MYSTERY_BOX_DELAY_MS)
    , renderer(renderSystem) {
        rng = std::default_random_engine(std::random_device()());


    // Submachine gun
    Gun submachineGun;
    StatModifier submachineStatModifier = StatModifier{
                            "Submachine Gun Stat",
                            false,              
                            0,              
                            0,
                            0,                 
                            1.1f,              
                            1.3f,              
                            1.3f,
                            0.0f               
                        };


    submachineGun.name = "Submachine Gun";
    submachineGun.statModifier = submachineStatModifier;
    submachineGun.fireRateMs = 100.0f;
    
    submachineGun.knockback = 800.0f;
    submachineGun.bulletVelocity = 1100.0f;
    submachineGun.distanceStrengthModifier = 1.2f;
    submachineGun.hasNormalDropOff = true;

    submachineGun.recoil = 20.0f;

    submachineGun.reloadMs = 1500.0f;
    
    submachineGun.magazineSize = 30;
    submachineGun.magazineAmmo = 30;
    submachineGun.reserveAmmo = submachineGun.magazineSize;
    submachineGun.gunSize = {60,30};

    // Assault rifle
    Gun assaultRifle;
    StatModifier assaultRifleStatModifier = StatModifier{
                            "Assault Rifle Stat",
                            false,              
                            0,              
                            0,
                            0,                
                            1.0f,              
                            0.90f,              
                            0.90f,
                            0.0f              
                        };


    assaultRifle.name = "Assault Rifle";
    assaultRifle.statModifier = assaultRifleStatModifier;
    assaultRifle.fireRateMs = 250.0f;
    
    assaultRifle.knockback = 1300.0f;
    assaultRifle.bulletVelocity = 1300.0f;
    assaultRifle.distanceStrengthModifier = 0.2f;
    assaultRifle.hasNormalDropOff = true;

    assaultRifle.recoil = 40.0f;

    assaultRifle.reloadMs = 2500.0f;
    
    assaultRifle.magazineSize = 20;
    assaultRifle.magazineAmmo = 20;
    assaultRifle.reserveAmmo = assaultRifle.magazineSize;

    assaultRifle.gunSize = {80,40};

    // Sniper rifle
    Gun sniperRifle;
    StatModifier sniperRifleStatModifier = StatModifier{
                            "Sniper Rifle Stat",
                            false,              
                            0,              
                            0, 
                            0,                
                            1.0f,              
                            0.85f,              
                            0.85f,
                            0.0f             
                        };


    sniperRifle.name = "Sniper Rifle";
    sniperRifle.statModifier = sniperRifleStatModifier;
    sniperRifle.fireRateMs = 650.0f;
    
    sniperRifle.knockback = 2500.0f;
    sniperRifle.bulletVelocity = 1800.0f;
    sniperRifle.distanceStrengthModifier = 1.5f;
    sniperRifle.hasNormalDropOff = false;

    sniperRifle.recoil = 150.0f;
    sniperRifle.recoilAnimationModifier = 0.8f;

    sniperRifle.reloadMs = 3500.0f;
    
    sniperRifle.magazineSize = 5;
    sniperRifle.magazineAmmo = 5;
    sniperRifle.reserveAmmo = sniperRifle.magazineSize;

    sniperRifle.gunSize = {120,30};

    // Sniper rifle
    Gun shotgun;
    StatModifier shotgunStatModifier = StatModifier{
                            "Shotgun Stat",
                            false,              
                            0,              
                            0, 
                            0,                
                            1.0f,              
                            1.2f,              
                            1.2f,
                            0.0f           
                        };


    shotgun.name = "Shotgun";
    shotgun.statModifier = shotgunStatModifier;
    shotgun.fireRateMs = 750.0f;
    
    shotgun.knockback = 3000.0f;
    shotgun.isHitScan = true;
    shotgun.bulletVelocity = 90.0f; // stores width of hit scan box
    shotgun.distanceStrengthModifier = 70.0f; // stores height of hitscan box

    shotgun.recoil = 200.0f;
    shotgun.recoilAnimationModifier = 0.8f;

    shotgun.reloadMs = 4000.0f;
    
    shotgun.magazineSize = 6;
    shotgun.magazineAmmo = 6;
    shotgun.reserveAmmo = shotgun.magazineSize;

    shotgun.gunSize = {90,30};

    guns.push_back(submachineGun);
    guns.push_back(assaultRifle);
    guns.push_back(sniperRifle);
    guns.push_back(shotgun);

}

void RandomDropsSystem::init(GameStateSystem* game_state_system)
{
    this->game_state_system = game_state_system;
    this->is_tutorial_intialized = false;
}

void RandomDropsSystem::step(float elapsed_ms_since_last_update)
{
    // Power up spawn
    next_powerup_spawn -= elapsed_ms_since_last_update;
    if (game_state_system->get_current_state() == 3 && next_powerup_spawn > TUTORIAL_POWERUP_DELAY_MS) {
        next_powerup_spawn = 1.f;
    }

    if (game_state_system->get_current_state() == 3 && !is_tutorial_intialized) {
        
        std::vector<int> powerup_positions = {200, 300, 400};

        int bottom_plat_pos_y = 633;

        if (next_powerup_spawn < 0.0f) {
            // Reset timer
            next_powerup_spawn = TUTORIAL_POWERUP_DELAY_MS;
            for (int i = 0; i < powerUpsNames.size(); i++) {
                std::string powerUpName = powerUpsNames.at(i);

                auto color = powerUpsNamesToColor[powerUpName];
                vec2 power_up_pos = { powerup_positions.at(i) , bottom_plat_pos_y - 5 - (int)(POWERUP_SIZE / 2) };

                bool should_spawn = true;

                for (int j = 0; j < registry.powerUps.components.size(); j++) {
                    if (powerUpName == registry.powerUps.components[j].statModifier.name) {
                        should_spawn = false;
                    }
                }

                if (should_spawn) {
                    auto statModifier = powerUpsNamesToStatModifier[powerUpName];
                    Entity entity = createPowerup(renderer, power_up_pos, { POWERUP_SIZE, POWERUP_SIZE }, color);
                    PowerUp& powerUp = registry.powerUps.emplace(entity);
                    powerUp.statModifier = statModifier;
                }
               
            }

            std::vector<int> gun_positions = { 700, 800, 900, 1000 };
            for (int i = 0; i < guns.size(); i++) {

                Gun gun = guns.at(i);

                bool should_spawn = true;

                for (int j = 0; j < registry.guns.components.size(); j++) {
                    if (gun.name == registry.guns.components[j].name) {
                        should_spawn = false;
                    }
                }
                if (should_spawn) {
                    vec2 mysteryBoxPos = { gun_positions.at(i), bottom_plat_pos_y - 5 - (int)(MYSTERY_BOX_SIZE / 2) };
                    Entity entity = createGunMysteryBox(renderer, mysteryBoxPos, { MYSTERY_BOX_SIZE, MYSTERY_BOX_SIZE });
                    GunMysteryBox& gunMysteryBox = registry.gunMysteryBoxes.emplace(entity);
                    gunMysteryBox.randomGun = gun;
                }
            }

            is_tutorial_intialized = true;
        }

        
    }
    else if (game_state_system->get_current_state() != 3)
    {
        if (registry.powerUps.components.size() >= MAX_POWERUPS) {
            next_powerup_spawn = POWERUP_DELAY_MS;
        }

        if (next_powerup_spawn < 0.0f) {
            // Reset timer
            next_powerup_spawn = (POWERUP_DELAY_MS / 2) + uniform_dist(rng) * (POWERUP_DELAY_MS / 2);

            vec2 powerUpPos = getRandomPositionOnPlatform();

            // Get random power up stat modifier
            int numPowerUps = powerUpsNames.size() - 1;
            int powerUpNum = uniform_dist_int(rng, decltype(uniform_dist_int)::param_type{ 0, numPowerUps });
            std::string powerUpName = powerUpsNames.at(powerUpNum);

            auto color = powerUpsNamesToColor[powerUpName];
            auto statModifier = powerUpsNamesToStatModifier[powerUpName];

            Entity entity = createPowerup(renderer, powerUpPos, { POWERUP_SIZE, POWERUP_SIZE }, color);

            PowerUp& powerUp = registry.powerUps.emplace(entity);
            powerUp.statModifier = statModifier;
        }


        next_mystery_box_spawn -= elapsed_ms_since_last_update;

        if (registry.gunMysteryBoxes.components.size() >= MAX_MYSTERY_BOX) {
            next_mystery_box_spawn = MYSTERY_BOX_DELAY_MS;
        }

        if (next_mystery_box_spawn < 0.0f) {
            // Reset timer
            next_mystery_box_spawn = (MYSTERY_BOX_DELAY_MS / 2) + uniform_dist(rng) * (MYSTERY_BOX_DELAY_MS / 2);

            vec2 mysteryBoxPos = getRandomPositionOnPlatform();

            // Get random gun
            int numGuns = guns.size() - 1;
            int gunNum = uniform_dist_int(rng, decltype(uniform_dist_int)::param_type{ 0, numGuns });

            Gun randomGun = guns.at(gunNum);

            Entity entity = createGunMysteryBox(renderer, mysteryBoxPos, { MYSTERY_BOX_SIZE, MYSTERY_BOX_SIZE });

            GunMysteryBox& gunMysteryBox = registry.gunMysteryBoxes.emplace(entity);
            gunMysteryBox.randomGun = randomGun;
        }
    }
}

vec2 RandomDropsSystem::getRandomPositionOnPlatform() {
    std::vector<Entity> platforms = registry.platforms.entities;

    int maxPlatformIndex = platforms.size() - 1;
    int platformNum = uniform_dist_int(rng, decltype(uniform_dist_int)::param_type{0, maxPlatformIndex});

    Entity randomPlatform = platforms.at(platformNum);

    Motion& platformMotion = registry.motions.get(randomPlatform);

    // Get random x coordinate along the platform
    int xCoord = uniform_dist_int(rng, decltype(uniform_dist_int)::param_type{(int) platformMotion.position.x - (int) (platformMotion.scale.x / 2) + (int) POWERUP_SIZE, (int) platformMotion.position.x + (int) (platformMotion.scale.x / 2) - (int) POWERUP_SIZE});
    int yCoord = (int) (platformMotion.position.y) - (int) (platformMotion.scale.y / 2) - (int) (POWERUP_SIZE / 2);

    return {xCoord, yCoord};
}

void RandomDropsSystem::handleInterpolation(float elapsed_ms) {
    if (game_state_system->get_current_state() != 3) {
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
}

