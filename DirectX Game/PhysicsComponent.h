#pragma once

#include <string>
#include <reactphysics3d/reactphysics3d.h>
#include "AComponent.h"

// PhysicsComponent depends on AComponent (which forward-declares GameObject),
// and uses reactphysics3d types. This header now ensures those types are known.

class PhysicsComponent : public AComponent {
public:
    PhysicsComponent(const std::string& name, float mass = 1000.0f, GameObject* owner = nullptr);
    ~PhysicsComponent() override;

    void perform(float deltaTime) override;
    reactphysics3d::RigidBody* getRigidBody() const;
    // Force rigid body transform from owner (useful when user moves object in editor)
    void syncOwnerToBody();
    // Copy rigid body transform into owner (used after physics step)
    void syncBodyToOwner();
    // Recreate collider to match owner's current scale/transform (call if scale changes at runtime)
    void recreateCollider();

    // Accessor for mass so it can be saved/loaded
    float getMass() const { return mass; }
    // Allow updating mass at runtime; this will update the underlying rigid body type/mass
    void setMass(float m);

private:
    float mass = 0.0f;
    reactphysics3d::RigidBody* rigidBody = nullptr;
    reactphysics3d::Collider* collider = nullptr;

    // Use a generic CollisionShape pointer and track which specific shape type
    reactphysics3d::CollisionShape* collisionShape = nullptr;

    // If we create a triangle mesh-based concave shape we need to keep the TriangleMesh pointer
    reactphysics3d::TriangleMesh* triangleMesh = nullptr;

    enum class ShapeKind { None = 0, Box = 1, ConcaveMesh = 2 };
    ShapeKind shapeKind = ShapeKind::None;

    void createColliderFromOwner(reactphysics3d::PhysicsCommon* physicsCommon, reactphysics3d::PhysicsWorld* physicsWorld);
};
