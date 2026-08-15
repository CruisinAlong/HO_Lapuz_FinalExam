#include "PlaneColliderComponent.h"
#include "BaseComponentSystem.h"
#include "PhysicsSystem.h"
#include "PhysicsComponent.h"
#include "GameObject.h"

using namespace reactphysics3d;

PlaneColliderComponent::PlaneColliderComponent(const std::string& name, GameObject* owner)
    : AComponent(name, AComponent::Physics, owner), m_collider(nullptr), m_boxShape(nullptr)
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
            comp = new PhysicsComponent("PhysicsAuto", 0.0f, getOwner());
            if (comp) getOwner()->attachComponent(comp);
        }
        if (comp && comp->getRigidBody()) {
            Vector3D sc = getOwner()->getScale();
            reactphysics3d::Vector3 halfExtents(sc.m_x * 0.5f, 0.01f, sc.m_z * 0.5f);
            m_boxShape = pc->createBoxShape(halfExtents);
            if (m_boxShape) {
                m_collider = comp->getRigidBody()->addCollider(m_boxShape, Transform::identity());
            }
        }
    }
}

PlaneColliderComponent::~PlaneColliderComponent()
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
    if (m_boxShape) {
        if (pc) pc->destroyBoxShape(m_boxShape);
        m_boxShape = nullptr;
    }
}

void PlaneColliderComponent::perform(float /*deltaTime*/) {
}
