#pragma once
#pragma once
#include "AComponent.h"
#include "Vector3D.h"
#include "GameObject.h"
#include <string>
#include <reactphysics3d/reactphysics3d.h>

class BoxColliderComponent : public AComponent {
public:
    BoxColliderComponent(const std::string& name, const Vector3D& halfExtents = Vector3D(0.5f,0.5f,0.5f), GameObject* owner = nullptr);
    ~BoxColliderComponent() override;

    void perform(float deltaTime) override;

    reactphysics3d::Collider* getCollider() const { return m_collider; }
    reactphysics3d::BoxShape* getShape() const { return m_boxShape; }

private:
    Vector3D m_halfExtents;
    reactphysics3d::Collider* m_collider = nullptr;
    reactphysics3d::BoxShape* m_boxShape = nullptr;
};
