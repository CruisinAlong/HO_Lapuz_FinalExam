#pragma once
#include "AComponent.h"
#include "Vector3D.h"
#include <reactphysics3d/reactphysics3d.h>

class CapsuleColliderComponent : public AComponent {
public:
    CapsuleColliderComponent(const std::string& name, float radius = 0.5f, float height = 2.0f, GameObject* owner = nullptr);
    ~CapsuleColliderComponent() override;

    void perform(float deltaTime) override;

    reactphysics3d::Collider* getCollider() const { return m_collider; }
    reactphysics3d::CapsuleShape* getShape() const { return m_capsuleShape; }

private:
    float m_radius;
    float m_height;
    reactphysics3d::Collider* m_collider = nullptr;
    reactphysics3d::CapsuleShape* m_capsuleShape = nullptr;
};
