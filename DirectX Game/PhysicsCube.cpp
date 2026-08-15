#include "PhysicsCube.h"
#include <cmath>
#include <cfloat>
#include "PhysicsPlane.h"
#include <algorithm>
#include "AComponent.h" // added to query attached components

PhysicsCube::PhysicsCube()
{
}

PhysicsCube::~PhysicsCube()
{
}

bool PhysicsCube::create()
{
    // Use base Cube creation
    return Cube::create();
}

void PhysicsCube::update(float dt)
{
    // If this GameObject has an attached Physics component, let the physics system
    // drive the transform. Avoid running the internal Euler integrator to prevent
    // conflicts between the custom integration and reactphysics3d.
    auto physicsComps = getComponentsOfType(AComponent::Physics);
    if (!physicsComps.empty()) {
        // Only call base to update visuals/constant buffers; do not modify position/velocity.
        Cube::update(dt);
        return;
    }

    if (!m_static) {
        // simple Euler integration (only used when no PhysicsComponent attached)
        m_velocity = m_velocity + m_gravity * dt;
        Vector3D pos = getPosition();
        pos = pos + m_velocity * dt;
        // Determine collision against any registered PhysicsPlane(s). Use the
        // highest plane that the cube penetrates (closest to the cube bottom).
        float groundY = -FLT_MAX;
        float cubeHalf = getScale().m_y * 0.5f;
        float bottom = pos.m_y - cubeHalf;
        const auto& planes = PhysicsPlane::getAllPlanes();
        for (auto p : planes) {
            if (!p) continue;
            // assume horizontal plane; use plane world Y position
            // Only consider this plane if the cube's X/Z lie within the plane's X/Z extents
            float planeY = p->getPosition().m_y;

            // Compute plane extents in X/Z from its scale (plane is centered at position)
            Vector3D planeScale = p->getScale();
            float halfX = std::fabs(planeScale.m_x) * 0.5f;
            float halfZ = std::fabs(planeScale.m_z) * 0.5f;

            // Cube horizontal position
            float cx = pos.m_x;
            float cz = pos.m_z;

            // Small tolerance to avoid numerical edge flicker
            const float eps = 1e-4f;

            bool insideX = (cx >= (p->getPosition().m_x - halfX - eps)) && (cx <= (p->getPosition().m_x + halfX + eps));
            bool insideZ = (cz >= (p->getPosition().m_z - halfZ - eps)) && (cz <= (p->getPosition().m_z + halfZ + eps));

            if (!insideX || !insideZ) {
                // cube is outside this plane's horizontal bounds — skip
                continue;
            }

            if (bottom < planeY && planeY > groundY) groundY = planeY;
        }

        if (groundY > -FLT_MAX) {
            if (bottom < groundY) {
                pos.m_y = groundY + cubeHalf;
                // simple restitution
                if (m_velocity.m_y < 0.0f) m_velocity.m_y = -m_velocity.m_y * 0.3f;
                // damp horizontal
                m_velocity.m_x *= 0.9f;
                m_velocity.m_z *= 0.9f;
                // small threshold to stop
                if (std::fabs(m_velocity.m_y) < 0.1f) m_velocity.m_y = 0.0f;
            }
        }
        setPosition(pos);
    }

    // call base update to keep any time-based animations and update constant buffer
    Cube::update(dt);
}
