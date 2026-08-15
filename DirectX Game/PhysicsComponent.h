#pragma once

#include <string>
#include <reactphysics3d/reactphysics3d.h>
#include "AComponent.h"


class PhysicsComponent : public AComponent {
public:
    PhysicsComponent(const std::string& name, float mass = 1000.0f, GameObject* owner = nullptr);
    ~PhysicsComponent() override;

    void perform(float deltaTime) override;
    reactphysics3d::RigidBody* getRigidBody() const;
    void syncOwnerToBody();
    void syncBodyToOwner();
    void recreateCollider();

    float getMass() const { return mass; }
    void setMass(float m);

private:
    float mass = 0.0f;
    reactphysics3d::RigidBody* rigidBody = nullptr;
    reactphysics3d::Collider* collider = nullptr;

    reactphysics3d::CollisionShape* collisionShape = nullptr;

    reactphysics3d::TriangleMesh* triangleMesh = nullptr;

    enum class ShapeKind { None = 0, Box = 1, ConcaveMesh = 2 };
    ShapeKind shapeKind = ShapeKind::None;

    void createColliderFromOwner(reactphysics3d::PhysicsCommon* physicsCommon, reactphysics3d::PhysicsWorld* physicsWorld);
};
