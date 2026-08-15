#pragma once
#pragma once
#include "AComponent.h"
#include "Vector3D.h"
#include <reactphysics3d/reactphysics3d.h>

class SphereColliderComponent : public AComponent {
public:
    SphereColliderComponent(const std::string& name, float radius = 0.5f, GameObject* owner = nullptr);
    ~SphereColliderComponent() override;

    void perform(float deltaTime) override;

    reactphysics3d::Collider* getCollider() const { return m_collider; }
    reactphysics3d::SphereShape* getShape() const { return m_sphereShape; }

private:
    float m_radius;
    reactphysics3d::Collider* m_collider = nullptr;
    reactphysics3d::SphereShape* m_sphereShape = nullptr;
};
