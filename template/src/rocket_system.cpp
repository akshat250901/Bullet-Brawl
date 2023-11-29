#include "rocket_system.hpp"
#include "world_init.hpp"

void RocketSystem::step(float elapsed_ms) {
    for (Entity entity : registry.rocket.entities) {
        Rocket& rocket = registry.rocket.get(entity);
        if (registry.bezierMotion.has(entity)) {
            updateBezierMotion(entity, elapsed_ms);
        }
    }
}

void RocketSystem::updateBezierMotion(Entity& entity, float deltaTime_ms) {
    BezierMotion& bezier = registry.bezierMotion.get(entity);
    Motion& motion = registry.motions.get(entity);
    float height = 80.0f;

    bezier.elapsedTime += deltaTime_ms;
    float t = bezier.elapsedTime / bezier.duration;

    if (motion.position.x - abs(motion.scale.x / 2) > window_width_px) {

        motion.position.x = motion.scale.x * -1;

        bezier.elapsedTime = 0;

        bezier.controlPoints = {
            motion.position,
            { motion.position.x + 70, motion.position.y - height },
            { motion.position.x + 140, motion.position.y + height },
            { motion.position.x + 230, motion.position.y}
	    };

        return;
    }

    if (t < 1.0f) {
        motion.position = interpolate(bezier.controlPoints, t);
    } else {
        bezier.elapsedTime = 0;

	    bezier.controlPoints = {
            motion.position,
            { motion.position.x + 70, motion.position.y - height },
            { motion.position.x + 140, motion.position.y + height },
            { motion.position.x + 230, motion.position.y}
	    };
    }
}

vec2 RocketSystem::interpolate(const std::vector<vec2>& points, float t) {

    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    // Cubic Bezier curve equation
    vec2 p = uuu * points[0]; // first term, with the first control point
    p += 3 * uu * t * points[1]; // second term, with the second control point
    p += 3 * u * tt * points[2]; // third term, with the third control point
    p += ttt * points[3]; // fourth term, with the fourth control point

    return p;
}