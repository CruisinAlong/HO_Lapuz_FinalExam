#include "BoxColliderComponent.h"
#include "BaseComponentSystem.h"
#include "PhysicsSystem.h"
#include "PhysicsComponent.h"
#include "GameObject.h"

using namespace reactphysics3d;

BoxColliderComponent::BoxColliderComponent(const std::string& name, const Vector3D& halfExtents, GameObject* owner)
    : AComponent(name, AComponent::Physics, owner), m_halfExtents(halfExtents), m_collider(nullptr), m_boxShape(nullptr)
{
    BaseComponentSystem* base = BaseComponentSystem::getInstance();
    if (!base) return;
    PhysicsSystem* phys = base->getPhysicsSystem();
    if (!phys) return;

    PhysicsCommon* pc = phys->getPhysicsCommon();
    PhysicsWorld* pw = phys->getPhysicsWorld();
    if (!pc || !pw) return;

    // If owner already has a PhysicsComponent, attach collider to its rigid body
    if (getOwner()) {
        auto comp = dynamic_cast<PhysicsComponent*>(getOwner()->findComponentOfType(AComponent::Physics));
        // If there's no PhysicsComponent yet, create one so we have a rigid body
        if (!comp) {
            comp = new PhysicsComponent("PhysicsAuto", 1.0f, getOwner());
            if (comp) getOwner()->attachComponent(comp);
        }
        if (comp && comp->getRigidBody()) {
            Vector3 half(m_halfExtents.m_x, m_halfExtents.m_y, m_halfExtents.m_z);
            m_boxShape = pc->createBoxShape(half);
            if (m_boxShape) {
                m_collider = comp->getRigidBody()->addCollider(m_boxShape, Transform::identity());
            }
        }
    }
}

BoxColliderComponent::~BoxColliderComponent()
{
    // Remove collider and shape
    BaseComponentSystem* base = BaseComponentSystem::getInstance();
    if (!base) return;
    PhysicsSystem* phys = base->getPhysicsSystem();
    if (!phys) return;
    PhysicsCommon* pc = phys->getPhysicsCommon();

    if (m_collider) {
        if (getOwner()) {
            auto comp = dynamic_cast<PhysicsComponent*>(getOwner()->findComponentOfType(AComponent::Physics));
            if (comp && comp->getRigidBody()) {
                comp->getRigidBody()->removeCollider(m_collider);
            }
        }
        m_collider = nullptr;
    }
    if (m_boxShape) {
        if (pc) pc->destroyBoxShape(m_boxShape);
        m_boxShape = nullptr;
    }
}

void BoxColliderComponent::perform(float /*deltaTime*/) {
    // Nothing to do per-frame for now
}
