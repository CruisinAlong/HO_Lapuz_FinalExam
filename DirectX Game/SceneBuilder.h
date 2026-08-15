#pragma once
#include "CorePrereqs.h"
#include "Prerequisites.h"
#include "Vector3D.h"
#include "AppWindow.h"

class GameObject;
class Texture;
class Mesh;

class SceneBuilder {
public:
    SceneBuilder() {}
    ~SceneBuilder() {}

    // Create basic cube (non-physics). If tex provided, attach MeshComponent with texture.
    ObjectInstance createCube(const Vector3D& pos = Vector3D(0.0f,0.0f,0.0f),
                              const Vector3D& rot = Vector3D(0.0f,0.0f,0.0f),
                              const Vector3D& scale = Vector3D(1.0f,1.0f,1.0f),
                              TexturePtr tex = TexturePtr());

    ObjectInstance createTexturedCube(TexturePtr tex,
                                      const Vector3D& pos = Vector3D(0.0f,0.0f,0.0f),
                                      const Vector3D& rot = Vector3D(0.0f,0.0f,0.0f),
                                      const Vector3D& scale = Vector3D(1.0f,1.0f,1.0f));

    ObjectInstance createPhysicsCube(float mass = 1.0f,
                                     const Vector3D& pos = Vector3D(0.0f,2.0f,0.0f),
                                     const Vector3D& rot = Vector3D(0.0f,0.0f,0.0f),
                                     const Vector3D& scale = Vector3D(1.0f,1.0f,1.0f),
                                     TexturePtr tex = TexturePtr());

    ObjectInstance createMeshInstanceFrom(MeshPtr mesh, const Vector3D& pos = Vector3D(0.0f,0.0f,0.0f), const Vector3D& scale = Vector3D(0.8f,0.8f,0.8f));

    ObjectInstance createCapsule(float height = 2.0f, const Vector3D& pos = Vector3D(0.0f,0.0f,0.0f));
    ObjectInstance createSphere(float radius = 0.5f, const Vector3D& pos = Vector3D(0.0f,0.0f,0.0f));

    ObjectInstance createPhysicsPlane(const Vector3D& pos = Vector3D(0.0f,-1.5f,0.0f), const Vector3D& scale = Vector3D(10.0f,0.1f,10.0f));

    // Convenience helpers that perform creation and push the resulting ObjectInstance
    // into the provided container. These let callers delegate spawn responsibilities
    // to SceneBuilder so AppWindow remains small.
    int addCubeTo(std::vector<ObjectInstance>& out, TexturePtr boxTexture = TexturePtr());
    int addPhysicsCubeTo(std::vector<ObjectInstance>& out, float mass = 1.0f, TexturePtr boxTexture = TexturePtr());
    int addTexturedCubeTo(std::vector<ObjectInstance>& out, TexturePtr tex);
    int addMeshInstanceTo(std::vector<ObjectInstance>& out, MeshPtr mesh, const Vector3D& pos = Vector3D(0.0f,0.0f,0.0f), const Vector3D& scale = Vector3D(0.8f,0.8f,0.8f));
    int addCapsuleTo(std::vector<ObjectInstance>& out, float height = 2.0f);
    int addSphereTo(std::vector<ObjectInstance>& out, float radius = 0.5f);
    int addPhysicsPlaneTo(std::vector<ObjectInstance>& out, const Vector3D& pos = Vector3D(0.0f,-1.5f,0.0f), const Vector3D& scale = Vector3D(10.0f,0.1f,10.0f));
};
