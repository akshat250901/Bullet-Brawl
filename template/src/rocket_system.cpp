#include "rocket_system.hpp"
#include "world_init.hpp"

void RocketSystem::step(float elapsed_ms) {
    for (Entity entity : registry.rocket.entities) {
        Rocket& rocket = registry.rocket.get(entity);
        rocket.timer = rocket.timer - elapsed_ms;
        if (rocket.timer <= 0) {
            registry.rocket.remove(entity);
            registry.motions.remove(entity);
            registry.bezierMotion.remove(entity);
            registry.simplePathFinding.remove(entity);
            registry.meshPtrs.remove(entity);
        }
        else {
            if (registry.bezierMotion.has(entity)) {
                updateBezierMotion(entity, elapsed_ms);
            }
            else if (registry.simplePathFinding.has(entity)) {
                updatePathfinding(entity, elapsed_ms);
            }
        }

    }
}

void RocketSystem::updateBezierMotion(Entity& entity, float deltaTime_ms) {
    BezierMotion& bezier = registry.bezierMotion.get(entity);
    Motion& motion = registry.motions.get(entity);

    bezier.elapsedTime += deltaTime_ms;
    float t = bezier.elapsedTime / bezier.duration;

    if (t < 1.0f) {
        motion.position = interpolate(bezier.controlPoints, t);
    }
    else {
        // Switch to pathfinding
        SimplePathfinding& pathfinding = registry.simplePathFinding.get(entity);
        pathfinding.active = true;
        registry.bezierMotion.remove(entity);
        updatePathfinding(entity, deltaTime_ms);
    }
}

void RocketSystem::updatePathfinding(Entity& entity, float deltaTime_ms) {
    SimplePathfinding& pathfinding = registry.simplePathFinding.get(entity);
    Motion& motion = registry.motions.get(entity);

    if (pathfinding.active) {
        if (registry.motions.has(pathfinding.targetEntity)) {
            Motion& targetMotion = registry.motions.get(pathfinding.targetEntity);
            vec2 direction = normalize(targetMotion.position - motion.position);
            motion.velocity = direction * pathfinding.speed;
            motion.position += motion.velocity * (deltaTime_ms / 1000.0f); // Convert ms to seconds
        }
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