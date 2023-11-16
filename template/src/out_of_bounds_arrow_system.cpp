// internal
#include "out_of_bounds_arrow_system.hpp"
#include "world_init.hpp"


void OutOfBoundsArrowSystem::step()
{
    auto& arrow_container = registry.outOfBoundsArrows;

    for (int i = 0; i < arrow_container.size(); i++) {
        OutOfBoundsArrow& arrow = arrow_container.components[i];
        Entity entity_i = arrow_container.entities[i];

        Motion& arrow_motion = registry.motions.get(entity_i);
        Motion& player_motion = registry.motions.get(arrow.entity_to_track);

        if (player_motion.position.y + abs(player_motion.scale.y / 2) >= 0.f &&
             player_motion.position.y - abs(player_motion.scale.y / 2) <= window_height_px && 
             player_motion.position.x + abs(player_motion.scale.x / 2) >= 0.f &&
             player_motion.position.x - abs(player_motion.scale.x / 2) <= window_width_px) {
                //player is within view boundaries and remove the arrow rendering
                registry.renderRequests.remove(entity_i);
                continue;
            }

        if (!registry.renderRequests.has(entity_i)) {
            registry.renderRequests.insert(
		    entity_i,
            { 	arrow.textureId,
                EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE });
        }

        const int offSet = 20;

        if (player_motion.position.y + abs(player_motion.scale.y / 2) < 0.f) { // player above screen
            // Set arrow to point up
            arrow_motion.angle = 0.f;

            arrow_motion.position.x = player_motion.position.x;
            arrow_motion.position.y = abs(arrow_motion.scale.y / 2) + offSet;

            continue;
        }

         if (player_motion.position.y - abs(player_motion.scale.y / 2) > window_height_px) { // player below screen
            // Set arrow to point down
            arrow_motion.angle = M_PI;

            arrow_motion.position.x = player_motion.position.x;
            arrow_motion.position.y = window_height_px - abs(arrow_motion.scale.y / 2) - offSet;
            
            continue;
        }

        if (player_motion.position.x + abs(player_motion.scale.x / 2) < 0.f) { // player to left of screen

            // Set arrow to point left
            arrow_motion.angle = -(M_PI / 2);

            arrow_motion.position.y = player_motion.position.y;
            arrow_motion.position.x = abs(arrow_motion.scale.y / 2) + offSet;
            
            continue;
        }

        if (player_motion.position.x - abs(player_motion.scale.x / 2) > window_width_px) { // player to right of screen
            // Set arrow to point right
            arrow_motion.angle = (M_PI / 2);

            arrow_motion.position.y = player_motion.position.y;
            arrow_motion.position.x = window_width_px - abs(arrow_motion.scale.y / 2) - offSet;
            continue;
        }
    }
}