#pragma once
#include "AComponent.h"
#include "Vector3D.h"
#include <reactphysics3d/reactphysics3d.h>

class PlaneColliderComponent : public AComponent {
public:
    PlaneColliderComponent(const std::string& name, GameObject* owner = nullptr);
    ~PlaneColliderComponent() override;

    void perform(float deltaTime) override;

    reactphysics3d::Collider* getCollider() const { return m_collider; }
    reactphysics3d::BoxShape* getShape() const { return m_boxShape; }

private:
    reactphysics3d::Collider* m_collider = nullptr;
    reactphysics3d::BoxShape* m_boxShape = nullptr;
};
