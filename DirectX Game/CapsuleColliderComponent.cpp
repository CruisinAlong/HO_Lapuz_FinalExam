#include "CapsuleColliderComponent.h"
#include "BaseComponentSystem.h"
#include "PhysicsSystem.h"
#include "PhysicsComponent.h"
#include "GameObject.h"

using namespace reactphysics3d;

CapsuleColliderComponent::CapsuleColliderComponent(const std::string& name, float radius, float height, GameObject* owner)
    : AComponent(name, AComponent::Physics, owner), m_radius(radius), m_height(height), m_collider(nullptr), m_capsuleShape(nullptr)
{
    BaseComponentSystem* base = BaseComponentSystem::getInstance();
    if (!base) return;
    PhysicsSystem* phys = base->getPhysicsSystem();
    if (!phys) return;

    PhysicsCommon* pc = phys->getPhysicsCommon();
    PhysicsWorld* pw = phys->getPhysicsWorld();
    if (!pc || !pw) return;

    if (getOwner()) {
        PhysicsComponent* comp = dynamic_cast<PhysicsComponent*>(getOwner()->findComponentOfType(AComponent::Physics));
        if (!comp) {
            comp = new PhysicsComponent("PhysicsAuto", 1.0f, getOwner());
            if (comp) getOwner()->attachComponent(comp);
        }
        if (comp && comp->getRigidBody()) {
            Vector3D sc = getOwner()->getScale();
            float scaledRadius = m_radius * ((sc.m_x + sc.m_z) * 0.5f);
            float scaledHeight = m_height * sc.m_y;
            m_capsuleShape = pc->createCapsuleShape(scaledRadius, scaledHeight);
            if (m_capsuleShape) {
                m_collider = comp->getRigidBody()->addCollider(m_capsuleShape, Transform::identity());
            }
        }
    }
}

CapsuleColliderComponent::~CapsuleColliderComponent()
{
    BaseComponentSystem* base = BaseComponentSystem::getInstance();
    if (!base) return;
    PhysicsSystem* phys = base->getPhysicsSystem();
    if (!phys) return;
    PhysicsCommon* pc = phys->getPhysicsCommon();

    if (m_collider) {
        if (getOwner()) {
            PhysicsComponent* comp = dynamic_cast<PhysicsComponent*>(getOwner()->findComponentOfType(AComponent::Physics));
            if (comp && comp->getRigidBody()) {
                comp->getRigidBody()->removeCollider(m_collider);
            }
        }
        m_collider = nullptr;
    }
    if (m_capsuleShape) {
        if (pc) pc->destroyCapsuleShape(m_capsuleShape);
        m_capsuleShape = nullptr;
    }
}

void CapsuleColliderComponent::perform(float /*deltaTime*/) {
}
