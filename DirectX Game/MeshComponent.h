#pragma once
#include "AComponent.h"
#include "GameObject.h"
#include "Mesh.h"
#include "Prerequisites.h" 
#include <string>
#include <string>
class Mesh;

class MeshComponent : public AComponent {
public:
    MeshComponent(const std::string& name, Mesh* meshPtr = nullptr, TexturePtr tex = TexturePtr(), GameObject* owner = nullptr)
        : AComponent(name, AComponent::MeshComp, owner), m_mesh(meshPtr), m_texture(tex) {}
    ~MeshComponent() override {}

    Mesh* getMesh() const { return m_mesh; }
    void setMesh(Mesh* m) { m_mesh = m; }

    TexturePtr getTexture() const { return m_texture; }
    void setTexture(TexturePtr t) { m_texture = t; }

    void perform(float /*deltaTime*/) override {}

private:
    Mesh* m_mesh = nullptr;
    TexturePtr m_texture;
};
