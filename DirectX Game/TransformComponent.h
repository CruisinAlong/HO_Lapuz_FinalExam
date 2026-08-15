#pragma once
#include "AComponent.h"
#include "Vector3D.h"

class TransformComponent : public AComponent {
public:
    TransformComponent(const std::string& name, AGameObject* owner = nullptr)
        : AComponent(name, AComponent::TransformComp, owner), position(), rotation(), scale(1.0f,1.0f,1.0f) {}
    ~TransformComponent() override {}

    void setPosition(const Vector3D& p) { position = p; }
    Vector3D getPosition() const { return position; }

    void setRotation(const Vector3D& r) { rotation = r; }
    Vector3D getRotation() const { return rotation; }

    void setScale(const Vector3D& s) { scale = s; }
    Vector3D getScale() const { return scale; }

    void perform(float /*deltaTime*/) override {}

private:
    Vector3D position;
    Vector3D rotation;
    Vector3D scale;
};
