#include "SphereColliderComponent.h"
#include "SphereColliderComponent.h"
#include "BaseComponentSystem.h"
#include "PhysicsSystem.h"
#include "PhysicsComponent.h"
#include "GameObject.h"

using namespace reactphysics3d;

SphereColliderComponent::SphereColliderComponent(const std::string& name, float radius, GameObject* owner)
    : AComponent(name, AComponent::Physics, owner), m_radius(radius), m_collider(nullptr), m_sphereShape(nullptr)
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
            m_sphereShape = pc->createSphereShape(scaledRadius);
            if (m_sphereShape) {
                m_collider = comp->getRigidBody()->addCollider(m_sphereShape, Transform::identity());
            }
        }
    }
}

SphereColliderComponent::~SphereColliderComponent()
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
    if (m_sphereShape) {
        if (pc) pc->destroySphereShape(m_sphereShape);
        m_sphereShape = nullptr;
    }
}

void SphereColliderComponent::perform(float /*deltaTime*/) {
}
