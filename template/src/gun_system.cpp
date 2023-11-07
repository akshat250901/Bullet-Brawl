#include "gun_system.hpp"
#include "sound_system.hpp"
#include "world_init.hpp"
#include "stat_util.cpp"
#include "create_gun_util.cpp"

void GunSystem::animateRecoil(Gun& gun_i, Motion& gun_motion, const Player& player_component) {
    float fireRateOriginalModified = gun_i.fireRateMs - (gun_i.fireRateMs * gun_i.recoilAnimationModifier);
    float fireRateTimerCurrentModified = gun_i.fireRateTimerMs - (gun_i.fireRateMs * gun_i.recoilAnimationModifier);

    float fireRateGunAnimate = fireRateOriginalModified / 2;
    float fireRateTimerAnimate = fireRateTimerCurrentModified;

    float maxAngleKick = (M_PI / 15);
    float maxRecoil = 5.0f;

    float radiansPerTimeMs = (maxAngleKick / fireRateGunAnimate);
    float distancePerTimeMs = (maxRecoil / fireRateGunAnimate);
    float timeRemaining = fireRateTimerAnimate - fireRateGunAnimate;

    // Calculate the recoil offsets
    float radiansOffset = timeRemaining > 0 ? (-radiansPerTimeMs * timeRemaining) + maxAngleKick : (radiansPerTimeMs * fireRateTimerAnimate);
    float recoilOffset = timeRemaining > 0 ? (-distancePerTimeMs * timeRemaining) + maxRecoil : (distancePerTimeMs * timeRemaining);

    // Apply the recoil depending on the direction the player is facing
    int directionMultiplier = player_component.facing_right ? -1 : 1;
    gun_motion.angle += directionMultiplier * radiansOffset;
    gun_motion.position.x += directionMultiplier * recoilOffset;
}

GunSystem::GunSystem(RenderSystem* renderSystem, SoundSystem* sound_system)
    : renderer(renderSystem), sound_system(sound_system) {
}

void GunSystem::step(float elapsed_ms_since_last_update) 
{
    // Iterate through all guns and handle all the timers and ammo
    for (uint i = 0; i < registry.guns.components.size(); i++) {
        Gun& gun_i = registry.guns.components[i];
        Entity entity_i = registry.guns.entities[i];
        Motion& gun_motion = registry.motions.get(entity_i);

        Entity owner = gun_i.gunOwner;
        Controller& controller = registry.controllers.get(owner);
        Motion& motion_player = registry.motions.get(owner);
        Player& player_component = registry.players.get(owner);

        if (player_component.facing_right) {
            gun_motion.position.x = motion_player.position.x + motion_player.scale.x / 2 - 5;
            gun_motion.position.y = motion_player.position.y;
        } else {
            gun_motion.position.x = motion_player.position.x - motion_player.scale.x / 2 + 5;
            gun_motion.position.y = motion_player.position.y;
        }
        // set angle to 0
        gun_motion.angle = 0.0f;
        

        bool fireKey = controller.fireKey;

        gun_i.fireRateTimerMs -= elapsed_ms_since_last_update;

        if (gun_i.currentlyReloading) {
            gun_i.reloadTimerMs -= elapsed_ms_since_last_update;

            if (gun_i.reloadTimerMs > 0) {
                if (gun_i.fireRateTimerMs - (gun_i.fireRateMs * gun_i.recoilAnimationModifier) < 0) {
                    continue;
                }

                animateRecoil(gun_i, gun_motion, player_component);
                continue;
            } 

            gun_i.currentlyReloading = false;
            gun_i.fireRateTimerMs = 0;

            if (!gun_i.hasInfiniteAmmo) {
                if (gun_i.reserveAmmo - gun_i.magazineSize >= 0) {
                    gun_i.reserveAmmo -= gun_i.magazineSize;
                    gun_i.magazineAmmo = gun_i.magazineSize;
                } else {
                    gun_i.magazineAmmo = gun_i.reserveAmmo;
                    gun_i.reserveAmmo = 0;
                }
            } else {
                gun_i.magazineAmmo = gun_i.magazineSize;
            }
        }

        // If on this line then not reloading
        if (gun_i.fireRateTimerMs > 0) {
            // handle recoil animation HERE
            
            // Gun recoil animation finishes at half the fire rate of the gun
            if (gun_i.fireRateTimerMs - (gun_i.fireRateMs * gun_i.recoilAnimationModifier) < 0) {
                continue;
            }

            animateRecoil(gun_i, gun_motion, player_component);
            continue;
        }

        //  If on this line then not reloading and gun can fire
        if (!fireKey) {
            gun_i.fireRateTimerMs = 0;
            continue;
        }

        // If on this line then player firing gun, not reloading, not between fire rate cool down
        gun_i.magazineAmmo -= 1;
        gun_i.fireRateTimerMs = gun_i.fireRateMs;

        // Apply recoil
        if (!player_component.facing_right) {
            motion_player.velocity.x += gun_i.recoil;
        } else {
            motion_player.velocity.x -= gun_i.recoil;
        }

        Entity bullet = createBullet(renderer, entity_i);

        sound_system->play_shoot_sound(gun_i.name);
        if (gun_i.magazineAmmo > 0) { 
            continue;
        }

        if (gun_i.reserveAmmo > 0) {
            // start reload
            gun_i.currentlyReloading = true;
            gun_i.reloadTimerMs = gun_i.reloadMs;
            printf("RELOADING??\n");
            sound_system->play_reload_sound(gun_i.name);
            continue;
        }

        // no ammo in gun drop it and bring back starting pistol

        StatModifier statModifier = gun_i.statModifier;
        StatUtil::remove_stat_modifier(player_component, statModifier);

        CreateGunUtil::givePlayerStartingPistol(renderer, owner, false);

        // Drop the used weapon
        registry.gravity.emplace(entity_i);
        registry.nonInteractables.emplace(entity_i);
        registry.guns.remove(entity_i);
    }
}