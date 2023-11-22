// internal
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

class StatUtil {
public:
    static void apply_stat_modifier(Player& playerToModify, const StatModifier& statModifier) {
        playerToModify.max_jumps += statModifier.extra_jumps;
        playerToModify.jump_force *= statModifier.jump_force_modifier;
        playerToModify.running_force *= statModifier.running_force_modifier;
        playerToModify.speed *= statModifier.max_speed_modifier;
        playerToModify.knockback_resistance -= statModifier.knockback_resistance_modifier;
    }

    static void remove_stat_modifier(Player& playerToModify, const StatModifier& statModifier) {
        playerToModify.max_jumps -= statModifier.extra_jumps;
        playerToModify.jump_force /= statModifier.jump_force_modifier;
        playerToModify.running_force /= statModifier.running_force_modifier;
        playerToModify.speed /= statModifier.max_speed_modifier;
        playerToModify.knockback_resistance += statModifier.knockback_resistance_modifier;
    }
};