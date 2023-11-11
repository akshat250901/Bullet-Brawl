#include "gun_system.hpp"
#include "sound_system.hpp"
#include "world_init.hpp"
#include "stat_util.cpp"
#include "create_gun_util.cpp"

// Checks if the two rectangles intersect
bool GunSystem::collidesRect(const Motion& motion1, const Motion& motion2)
{
    // Calculate the half width and half height for each rectangle
    float halfWidth1 = motion1.scale.x / 2.0f;
    float halfHeight1 = motion1.scale.y / 2.0f;
    float halfWidth2 = motion2.scale.x / 2.0f;
    float halfHeight2 = motion2.scale.y / 2.0f;

    // Check for overlap in the x-axis
    if (abs(motion1.position.x - motion2.position.x) < (halfWidth1 + halfWidth2)) {
        // Check for overlap in the y-axis
        if (abs(motion1.position.y - motion2.position.y) < (halfHeight1 + halfHeight2)) {
            return true; // The rectangles intersect
        }
    }
    return false; // The rectangles don't intersect
}

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

void GunSystem:: checkHitscanCollision(Gun& gun, Motion& hitscan_motion, Player& player_component) {
    auto& motion_container = registry.motions;
    auto& players_container = registry.players;

    // Check for collisions between players and mystery boxes
	for(uint i = 0; i < players_container.components.size(); i++)
	{
		Entity entity_i = players_container.entities[i];
		Motion& motion_i = motion_container.get(entity_i);
		
		if (entity_i != gun.gunOwner && collidesRect(motion_i, hitscan_motion))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity

                auto entity_j = Entity();

                Bullet& bullet = registry.bullets.emplace(entity_j);
 	            bullet.shooter = gun.gunOwner;
                bullet.isHitscan = true;
	            bullet.knockback = player_component.facing_right == 1 ? gun.knockback : -1 * gun.knockback;

				registry.playerBulletCollisions.emplace_with_duplicates(entity_i, entity_j);
				registry.playerBulletCollisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
}


GunSystem::GunSystem(RenderSystem* renderSystem, SoundSystem* sound_system)
    : renderer(renderSystem), sound_system(sound_system) {
}

void GunSystem::step(float elapsed_ms_since_last_update) 
{

    // Iterate through all muzzle flashes and remove if timer is below 0

    for (uint i = 0; i < registry.muzzleFlashes.components.size(); i++) {
         Entity entity_i = registry.muzzleFlashes.entities[i];
         MuzzleFlash& component_i = registry.muzzleFlashes.components[i];

         if (component_i.timerMs >= 0) {
            component_i.timerMs -= elapsed_ms_since_last_update;
         } else {
            registry.remove_all_components_of(entity_i);
         }
    }

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

        sound_system->play_shoot_sound(gun_i.name);

        if (!gun_i.isHitScan) {
            Entity bullet = createBullet(renderer, entity_i);
        } else {
            // Handle hitscan guns
            float lengthOfHitscan = gun_i.bulletVelocity;
            float heightOfHitscan = gun_i.distanceStrengthModifier;

            Motion hitscan_motion;

            float xPositionHitscan = player_component.facing_right == 1 ? gun_motion.position.x + (gun_motion.scale.x / 2) + (lengthOfHitscan / 2) : gun_motion.position.x - gun_motion.scale.x / 2 - (lengthOfHitscan / 2);

            hitscan_motion.scale = {lengthOfHitscan, heightOfHitscan};
            hitscan_motion.position = {xPositionHitscan, gun_motion.position.y };

            createMuzzleFlash(renderer, hitscan_motion);
            checkHitscanCollision(gun_i, hitscan_motion, player_component);
        }


        if (gun_i.magazineAmmo > 0) { 
            continue;
        }

        if (gun_i.reserveAmmo > 0) {
            // start reload
            gun_i.currentlyReloading = true;
            gun_i.reloadTimerMs = gun_i.reloadMs;
            sound_system->play_reload_sound(gun_i.name);
            continue;
        }

        // no ammo in gun drop it and bring back starting pistol

        if (!player_component.facing_right) {
            gun_motion.scale.x *= -1; // flip gun
        }

        StatModifier statModifier = gun_i.statModifier;
        StatUtil::remove_stat_modifier(player_component, statModifier);

        CreateGunUtil::givePlayerStartingPistol(renderer, owner, false);

        // Drop the used weapon
        registry.gravity.emplace(entity_i);
        registry.nonInteractables.emplace(entity_i);
        registry.guns.remove(entity_i);
    }
}