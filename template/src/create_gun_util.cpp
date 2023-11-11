// internal
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "world_init.hpp"

class CreateGunUtil {
public:
    static void givePlayerStartingPistol(RenderSystem* renderer, Entity owner, bool removePrevious) {

        if (removePrevious) {
             // Removes previous gun if found and restores stats
            for (uint i = 0; i < registry.guns.components.size(); i++) {
                Gun& gun_i = registry.guns.components[i];

                if (gun_i.gunOwner == owner) {
                    StatModifier statModifier = gun_i.statModifier;
                    Player& player = registry.players.get(owner);

                    player.max_jumps -= statModifier.extra_jumps;
                    player.jump_force /= statModifier.jump_force_modifier;
                    player.running_force /= statModifier.running_force_modifier;
                    player.speed /= statModifier.max_speed_modifier;

                    registry.remove_all_components_of(registry.guns.entities[i]);

                    break;
                }
            }
        }

        StatModifier defaultStatModifier;

        Entity gunEntity = createGun(renderer, {30, 30}, "Pistol");

        Gun& gunComponent = registry.guns.emplace(gunEntity);
	    gunComponent.gunOwner = owner;
	    gunComponent.statModifier = defaultStatModifier;
	    gunComponent.hasInfiniteAmmo = true;
    }
};