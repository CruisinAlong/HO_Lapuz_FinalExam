#include "PhysicsCube.h"
#include <cmath>
#include <cfloat>
#include "PhysicsPlane.h"
#include <algorithm>
#include "AComponent.h" 

PhysicsCube::PhysicsCube()
{
}

PhysicsCube::~PhysicsCube()
{
}

bool PhysicsCube::create()
{
    return Cube::create();
}

void PhysicsCube::update(float dt)
{

    auto physicsComps = getComponentsOfType(AComponent::Physics);
    if (!physicsComps.empty()) {
        Cube::update(dt);
        return;
    }

    if (!m_static) {
        m_velocity = m_velocity + m_gravity * dt;
        Vector3D pos = getPosition();
        pos = pos + m_velocity * dt;

        float groundY = -FLT_MAX;
        float cubeHalf = getScale().m_y * 0.5f;
        float bottom = pos.m_y - cubeHalf;
        const auto& planes = PhysicsPlane::getAllPlanes();
        for (auto p : planes) {
            if (!p) continue;

            float planeY = p->getPosition().m_y;


            Vector3D planeScale = p->getScale();
            float halfX = std::fabs(planeScale.m_x) * 0.5f;
            float halfZ = std::fabs(planeScale.m_z) * 0.5f;

            float cx = pos.m_x;
            float cz = pos.m_z;

            const float eps = 1e-4f;

            bool insideX = (cx >= (p->getPosition().m_x - halfX - eps)) && (cx <= (p->getPosition().m_x + halfX + eps));
            bool insideZ = (cz >= (p->getPosition().m_z - halfZ - eps)) && (cz <= (p->getPosition().m_z + halfZ + eps));

            if (!insideX || !insideZ) {
                continue;
            }

            if (bottom < planeY && planeY > groundY) groundY = planeY;
        }

        if (groundY > -FLT_MAX) {
            if (bottom < groundY) {
                pos.m_y = groundY + cubeHalf;
                if (m_velocity.m_y < 0.0f) m_velocity.m_y = -m_velocity.m_y * 0.3f;
                m_velocity.m_x *= 0.9f;
                m_velocity.m_z *= 0.9f;
                if (std::fabs(m_velocity.m_y) < 0.1f) m_velocity.m_y = 0.0f;
            }
        }
        setPosition(pos);
    }

    Cube::update(dt);
}
