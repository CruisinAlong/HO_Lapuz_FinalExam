#include "SceneBuilder.h"
#include "Cube.h"
#include "PhysicsCube.h"
#include "PhysicsPlane.h"
#include "Capsule.h"
#include "Sphere.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "MeshManager.h"
#include "Texture.h"
#include "PhysicsComponent.h"
#include "BoxColliderComponent.h"
#include "GraphicsEngine.h"
#include "ShaderLibrary.h"
#include "Debug.h"

// createCube
ObjectInstance SceneBuilder::createCube(const Vector3D& pos, const Vector3D& rot, const Vector3D& scale, TexturePtr tex)
{
    ObjectInstance out;
    Cube* c = new Cube();
    if (!c) return out;
    if (!c->create()) { c->destroy(); delete c; return out; }

    c->setPosition(pos);
    c->setRotation(rot);
    c->setScale(scale);

    MeshComponent* mc = new MeshComponent("MeshComp", nullptr, tex, c);
    if (mc) c->attachComponent(mc);

    out.object = c;
    out.position = pos;
    out.rotation = rot;
    out.scale = scale;
    return out;
}

// createSphere
ObjectInstance SceneBuilder::createSphere(float radius, const Vector3D& pos)
{
    ObjectInstance out;
    LOG_DEBUG("SceneBuilder: createSphere() - requested radius=%.3f pos=(%.3f,%.3f,%.3f)",
              radius, pos.m_x, pos.m_y, pos.m_z);

    // Ensure shared GPU resources and instance buffer for Sphere are initialized
    auto ge = GraphicsEngine::getInstance();
    if (ge) {
        auto rs = ge->getRenderSystem();
        if (rs) {
            bool okShared = Sphere::InitSharedResources(rs, 16, 16);
            LOG_DEBUG("SceneBuilder: Sphere::InitSharedResources returned %s", okShared ? "true" : "false");
            bool okInst = Sphere::InitInstanceBuffer(rs, 10000);
            LOG_DEBUG("SceneBuilder: Sphere::InitInstanceBuffer returned %s", okInst ? "true" : "false");
        } else {
            LOG_DEBUG("SceneBuilder: no RenderSystem available for Sphere init");
        }
    } else {
        LOG_DEBUG("SceneBuilder: no GraphicsEngine instance available for Sphere init");
    }

    Sphere* s = new Sphere();
    if (!s) return out;
    if (!s->create(16,16)) { s->destroy(); delete s; return out; }

    s->setScale(Vector3D(radius*2.0f, radius*2.0f, radius*2.0f));
    s->setPosition(pos);
    s->setRotation(Vector3D(0.0f,0.0f,0.0f));

    // Attach a MeshComponent so the Sphere is visible when spawned. No collider/physics
    // is added here; colliders may be created/attached later via editor or code.
    MeshComponent* smc = new MeshComponent("MeshComp", nullptr, TexturePtr(), s);
    if (smc) s->attachComponent(smc);

    // Log world translation & scale (diagnostic for transform errors)
    Matrix4x4 world = s->getWorldMatrix();
    Vector3D tr = world.getTranslation();
    Vector3D sc = s->getScale();
    LOG_DEBUG("SceneBuilder: Sphere world translation=(%.3f,%.3f,%.3f) scale=(%.3f,%.3f,%.3f)",
              tr.m_x, tr.m_y, tr.m_z, sc.m_x, sc.m_y, sc.m_z);

    out.object = s;
    out.position = pos;
    out.rotation = s->getRotation();
    out.scale = s->getScale();
    return out;
}

// createTexturedCube
ObjectInstance SceneBuilder::createTexturedCube(TexturePtr tex, const Vector3D& pos, const Vector3D& rot, const Vector3D& scale)
{
    return createCube(pos, rot, scale, tex);
}

int SceneBuilder::addSphereTo(std::vector<ObjectInstance>& out, float radius)
{
    ObjectInstance ci = createSphere(radius, Vector3D(0.0f,0.0f,0.0f));
    if (!ci.object) return -1;
    out.push_back(ci);
    return static_cast<int>(out.size() - 1);
}

// createPhysicsCube
ObjectInstance SceneBuilder::createPhysicsCube(float mass, const Vector3D& pos, const Vector3D& rot, const Vector3D& scale, TexturePtr tex)
{
    ObjectInstance out;
    PhysicsCube* pcube = new PhysicsCube();
    if (!pcube) return out;
    if (!pcube->create()) { pcube->destroy(); delete pcube; return out; }

    pcube->setPosition(pos);
    pcube->setRotation(rot);
    pcube->setScale(scale);

    MeshComponent* mc = new MeshComponent("MeshComp", nullptr, tex, pcube);
    if (mc) pcube->attachComponent(mc);

    PhysicsComponent* phys = new PhysicsComponent("PhysicsCube", mass, pcube);
    if (phys) {
        if (!pcube->findComponentOfType(AComponent::Physics)) {
            pcube->attachComponent(phys);
            phys->recreateCollider();
            phys->syncOwnerToBody();
            reactphysics3d::Vector3 zero(0.0f,0.0f,0.0f);
            if (phys->getRigidBody()) {
                phys->getRigidBody()->setLinearVelocity(zero);
                phys->getRigidBody()->setAngularVelocity(zero);
            }
        } else {
            delete phys;
        }
    }

    out.object = pcube;
    out.position = pos;
    out.rotation = rot;
    out.scale = scale;
    return out;
}

// createMeshInstanceFrom
ObjectInstance SceneBuilder::createMeshInstanceFrom(MeshPtr mesh, const Vector3D& pos, const Vector3D& scale)
{
    ObjectInstance out;
    Cube* c = new Cube();
    if (!c) return out;
    if (!c->create()) { c->destroy(); delete c; return out; }

    VertexBuffer* vb = mesh->getVertexBuffer();
    IndexBuffer* ib = mesh->getIndexBuffer();
    if (vb && ib) c->setBuffers(vb, ib);

    ShaderLibrary* lib = ShaderLibrary::getInstance();
    if (lib) {
        VertexShader* vs = lib->getVertexShader(ShaderNames::TEX_VS);
        PixelShader* ps = lib->getPixelShader(ShaderNames::TEX_PS);
        if (vs && ps) c->setShaders(vs, ps);
    }

    MeshComponent* mc = new MeshComponent("MeshComp", mesh.get(), TexturePtr(), c);
    if (mc) c->attachComponent(mc);

    c->setPosition(pos);
    c->setRotation(Vector3D(0.0f,0.0f,0.0f));
    c->setScale(scale);

    PhysicsComponent* pc = new PhysicsComponent("PhysicsCube", 1.0f, c);
    if (pc) c->attachComponent(pc);

    out.object = c;
    out.position = pos;
    out.rotation = c->getRotation();
    out.scale = scale;
    return out;
}

// createCapsule
ObjectInstance SceneBuilder::createCapsule(float height, const Vector3D& pos)
{
    ObjectInstance out;
    LOG_DEBUG("SceneBuilder: createCapsule() - requested height=%.3f pos=(%.3f,%.3f,%.3f)", height, pos.m_x, pos.m_y, pos.m_z);
    // Ensure shared GPU resources for Capsule are initialized (if a render system is available)
    {
        auto ge = GraphicsEngine::getInstance();
        if (ge) {
            auto rs = ge->getRenderSystem();
            if (rs) {
                bool okShared = Capsule::InitSharedResources(rs, 16, 16);
                LOG_DEBUG("SceneBuilder: Capsule::InitSharedResources returned %s", okShared ? "true" : "false");
                bool okInst = Capsule::InitInstanceBuffer(rs, 10000);
                LOG_DEBUG("SceneBuilder: Capsule::InitInstanceBuffer returned %s", okInst ? "true" : "false");
            } else {
                LOG_DEBUG("SceneBuilder: no RenderSystem available for Capsule::InitSharedResources");
            }
        } else {
            LOG_DEBUG("SceneBuilder: no GraphicsEngine instance available for Capsule::InitSharedResources");
        }
    }

    Capsule* cap = new Capsule();
    if (!cap) return out;
    if (!cap->create(16,16)) { cap->destroy(); delete cap; return out; }
    LOG_DEBUG("SceneBuilder: Capsule created successfully");
    cap->setScale(Vector3D(1.0f, height, 1.0f));
    cap->setPosition(pos);
    cap->setRotation(Vector3D(0.0f,0.0f,0.0f));

    // Attach a MeshComponent so the Capsule is visible when spawned. Colliders are
    // intentionally not attached here; they can be added later if desired.
    MeshComponent* cmc = new MeshComponent("MeshComp", nullptr, TexturePtr(), cap);
    if (cmc) cap->attachComponent(cmc);

    // Log world translation & scale (diagnostic for transform errors)
    Matrix4x4 world = cap->getWorldMatrix();
    Vector3D tr = world.getTranslation();
    Vector3D sc = cap->getScale();
    LOG_DEBUG("SceneBuilder: Capsule world translation=(%.3f,%.3f,%.3f) scale=(%.3f,%.3f,%.3f)",
              tr.m_x, tr.m_y, tr.m_z, sc.m_x, sc.m_y, sc.m_z);

    out.object = cap;
    out.position = pos;
    out.rotation = cap->getRotation();
    out.scale = cap->getScale();
    return out;
}

// createPhysicsPlane
ObjectInstance SceneBuilder::createPhysicsPlane(const Vector3D& pos, const Vector3D& scale)
{
    ObjectInstance out;
    LOG_DEBUG("SceneBuilder: createPhysicsPlane() - requested pos=(%.3f,%.3f,%.3f) scale=(%.3f,%.3f,%.3f)", pos.m_x, pos.m_y, pos.m_z, scale.m_x, scale.m_y, scale.m_z);
    // Ensure shared GPU resources for Plane are initialized (if a render system is available)
    {
        auto ge = GraphicsEngine::getInstance();
        if (ge) {
            auto rs = ge->getRenderSystem();
            if (rs) {
                bool ok = Plane::InitSharedResources(rs);
                LOG_DEBUG("SceneBuilder: Plane::InitSharedResources returned %s", ok ? "true" : "false");
                bool okInst = Plane::InitInstanceBuffer(rs, 10000);
                LOG_DEBUG("SceneBuilder: Plane::InitInstanceBuffer returned %s", okInst ? "true" : "false");
            } else {
                LOG_DEBUG("SceneBuilder: no RenderSystem available for Plane::InitSharedResources");
            }
        } else {
            LOG_DEBUG("SceneBuilder: no GraphicsEngine instance available for Plane::InitSharedResources");
        }
    }

    PhysicsPlane* p = new PhysicsPlane();
    if (!p) return out;
    if (!p->create()) { p->destroy(); delete p; return out; }
    LOG_DEBUG("SceneBuilder: PhysicsPlane created successfully");
    p->setPosition(pos);
    p->setRotation(Vector3D(0.0f,0.0f,0.0f));
    p->setScale(scale);

    PhysicsComponent* pc = new PhysicsComponent("PhysicsPlane", 0.0f, p);
    if (pc) {
        if (pc->getRigidBody()) pc->getRigidBody()->setType(reactphysics3d::BodyType::STATIC);
        p->attachComponent(pc);
    }

    // Log world translation & scale (diagnostic for transform errors)
    Matrix4x4 world = p->getWorldMatrix();
    Vector3D tr = world.getTranslation();
    Vector3D sc = p->getScale();
    LOG_DEBUG("SceneBuilder: Plane world translation=(%.3f,%.3f,%.3f) scale=(%.3f,%.3f,%.3f)",
              tr.m_x, tr.m_y, tr.m_z, sc.m_x, sc.m_y, sc.m_z);

    out.object = p;
    out.position = pos;
    out.rotation = p->getRotation();
    out.scale = p->getScale();
    return out;
}

int SceneBuilder::addCubeTo(std::vector<ObjectInstance>& out, TexturePtr boxTexture)
{
    ObjectInstance ci = createCube(Vector3D(0.0f,0.0f,0.0f), Vector3D(0.0f,0.0f,0.0f), Vector3D(1.0f,1.0f,1.0f), boxTexture);
    if (!ci.object) return -1;
    out.push_back(ci);
    int idx = static_cast<int>(out.size() - 1);
    // Ensure physics component exists
    auto comps = ci.object->getComponentsOfType(AComponent::Physics);
    if (comps.empty()) {
        PhysicsComponent* pc = new PhysicsComponent("PhysicsCube", 1.0f, ci.object);
        if (pc) { ci.object->attachComponent(pc); pc->syncOwnerToBody(); }
    }
    return idx;
}

int SceneBuilder::addPhysicsCubeTo(std::vector<ObjectInstance>& out, float mass, TexturePtr boxTexture)
{
    ObjectInstance ci = createPhysicsCube(mass, Vector3D(0.0f,2.0f,0.0f), Vector3D(0.0f,0.0f,0.0f), Vector3D(1.0f,1.0f,1.0f), boxTexture);
    if (!ci.object) return -1;
    out.push_back(ci);
    return static_cast<int>(out.size() - 1);
}

int SceneBuilder::addTexturedCubeTo(std::vector<ObjectInstance>& out, TexturePtr tex)
{
    ObjectInstance ci = createTexturedCube(tex, Vector3D(0.0f,0.0f,0.0f));
    if (!ci.object) return -1;
    out.push_back(ci);
    int idx = static_cast<int>(out.size() - 1);
    auto comps = ci.object->getComponentsOfType(AComponent::Physics);
    if (comps.empty()) {
        PhysicsComponent* pc = new PhysicsComponent("PhysicsCube", 1.0f, ci.object);
        if (pc) ci.object->attachComponent(pc);
    }
    return idx;
}

int SceneBuilder::addMeshInstanceTo(std::vector<ObjectInstance>& out, MeshPtr mesh, const Vector3D& pos, const Vector3D& scale)
{
    ObjectInstance ci = createMeshInstanceFrom(mesh, pos, scale);
    if (!ci.object) return -1;
    out.push_back(ci);
    return static_cast<int>(out.size() - 1);
}

int SceneBuilder::addCapsuleTo(std::vector<ObjectInstance>& out, float height)
{
    ObjectInstance ci = createCapsule(height, Vector3D(0.0f,0.0f,0.0f));
    if (!ci.object) return -1;
    out.push_back(ci);
    return static_cast<int>(out.size() - 1);
}

int SceneBuilder::addPhysicsPlaneTo(std::vector<ObjectInstance>& out, const Vector3D& pos, const Vector3D& scale)
{
    ObjectInstance ci = createPhysicsPlane(pos, scale);
    if (!ci.object) return -1;
    out.push_back(ci);
    int idx = static_cast<int>(out.size() - 1);
    // Mark rigid body static if present
    auto comps = ci.object->getComponentsOfType(AComponent::Physics);
    for (auto c : comps) {
        if (!c) continue;
        PhysicsComponent* pc = dynamic_cast<PhysicsComponent*>(c);
        if (pc && pc->getRigidBody()) pc->getRigidBody()->setType(reactphysics3d::BodyType::STATIC);
    }
    return idx;
}
