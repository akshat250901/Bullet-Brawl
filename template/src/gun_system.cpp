#include "gun_system.hpp"
#include "world_init.hpp"
#include "stat_util.cpp"
#include "create_gun_util.cpp"

GunSystem::GunSystem(RenderSystem* renderSystem) 
    : renderer(renderSystem) {

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
        

        bool fireKey = controller.fireKey;

        if (gun_i.currentlyReloading) {
            gun_i.reloadTimerMs -= elapsed_ms_since_last_update;

            if (gun_i.reloadTimerMs > 0) {
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

        gun_i.fireRateTimerMs -= elapsed_ms_since_last_update;

        // If on this line then not reloading
        if (gun_i.fireRateTimerMs > 0) {
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

        if (gun_i.magazineAmmo > 0) { 
            continue;
        }

        if (gun_i.reserveAmmo > 0) {
            // start reload
            gun_i.currentlyReloading = true;
            gun_i.reloadTimerMs = gun_i.reloadMs;
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