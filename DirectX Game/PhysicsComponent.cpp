#include "PhysicsComponent.h"
#include "GameObject.h"
#include "BaseComponentSystem.h"
#include "PhysicsSystem.h"
#include "Matrix4x4.h"
#include "CorePrereqs.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#include "PhysicsPlane.h" 

using namespace reactphysics3d;

static Quaternion eulerToQuaternion(const Vector3D& euler)
{
    float cx = std::cosf(euler.m_x * 0.5f);
    float sx = std::sinf(euler.m_x * 0.5f);
    float cy = std::cosf(euler.m_y * 0.5f);
    float sy = std::sinf(euler.m_y * 0.5f);
    float cz = std::cosf(euler.m_z * 0.5f);
    float sz = std::sinf(euler.m_z * 0.5f);

    Quaternion qx(sx, 0, 0, cx);
    Quaternion qy(0, sy, 0, cy);
    Quaternion qz(0, 0, sz, cz);
    return qy * qx * qz;
}

void PhysicsComponent::createColliderFromOwner(PhysicsCommon* physicsCommon, PhysicsWorld* physicsWorld)
{
    if (!physicsCommon || !physicsWorld || !getOwner() || !rigidBody) return;

    if (collider) {
        rigidBody->removeCollider(collider);
        collider = nullptr;
    }

    if (collisionShape) {
        switch (shapeKind) {
        case ShapeKind::Box:
            physicsCommon->destroyBoxShape(static_cast<BoxShape*>(collisionShape));
            break;
        case ShapeKind::ConcaveMesh:
            physicsCommon->destroyConcaveMeshShape(static_cast<ConcaveMeshShape*>(collisionShape));
            if (triangleMesh) {
                physicsCommon->destroyTriangleMesh(triangleMesh);
                triangleMesh = nullptr;
            }
            break;
        default:
            break;
        }
        collisionShape = nullptr;
        shapeKind = ShapeKind::None;
    }

    if (dynamic_cast<PhysicsPlane*>(getOwner())) {
        const float halfExtent = 0.5f;
        float vertices[4 * 3] = {
            -halfExtent, 0.0f, -halfExtent,  
            -halfExtent, 0.0f,  halfExtent,  
             halfExtent, 0.0f, -halfExtent,  
             halfExtent, 0.0f,  halfExtent   
        };
        uint32_t indices[2 * 3] = {
            0, 1, 2,
            1, 3, 2
        };

        TriangleVertexArray triArray(
            4,
            vertices, sizeof(float) * 3,
            2,
            indices, sizeof(uint32_t) * 3,
            TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
            TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE
        );

        std::vector<Message> messages;
        TriangleMesh* triMesh = physicsCommon->createTriangleMesh(triArray, messages);
        if (triMesh) {
            Vector3D s = getOwner()->getScale();
            Vector3 scaling(s.m_x, s.m_y, s.m_z);
            ConcaveMeshShape* concave = physicsCommon->createConcaveMeshShape(triMesh, scaling);
            if (concave) {
                collisionShape = concave;
                triangleMesh = triMesh;
                shapeKind = ShapeKind::ConcaveMesh;
                collider = rigidBody->addCollider(collisionShape, Transform::identity());
                rigidBody->setType(BodyType::STATIC);
                return;
            } else {
                physicsCommon->destroyTriangleMesh(triMesh);
            }
        }
    }

    Vector3D s = getOwner()->getScale();
    float hx = std::max(0.0001f, std::fabs(s.m_x) * 0.5f);
    float hy = std::max(0.0001f, std::fabs(s.m_y) * 0.5f);
    float hz = std::max(0.0001f, std::fabs(s.m_z) * 0.5f);

    Vector3 half(hx, hy, hz);
    BoxShape* box = physicsCommon->createBoxShape(half);
    if (box) {
        collisionShape = box;
        shapeKind = ShapeKind::Box;
        collider = rigidBody->addCollider(collisionShape, Transform::identity());
    }

    rigidBody->updateMassPropertiesFromColliders();
    if (mass > 0.0f) {
        rigidBody->setMass(mass);
        rigidBody->setType(BodyType::DYNAMIC);
    } else {
        rigidBody->setType(BodyType::STATIC);
    }
}

PhysicsComponent::PhysicsComponent(const std::string& name, float mass_, GameObject* owner)
    : AComponent(name, AComponent::Physics, owner), mass(mass_), rigidBody(nullptr), collider(nullptr), collisionShape(nullptr), triangleMesh(nullptr), shapeKind(ShapeKind::None) {
    BaseComponentSystem* base = BaseComponentSystem::getInstance();
    if (!base) return;

    PhysicsSystem* physSys = base->getPhysicsSystem();
    if (!physSys) return;


    {
        std::string uniqueSuffix = "_" + std::to_string(reinterpret_cast<uintptr_t>(this));
        this->name += uniqueSuffix;
    }

    physSys->registerComponent(this);


    PhysicsCommon* physicsCommon = physSys->getPhysicsCommon();
    PhysicsWorld* physicsWorld = physSys->getPhysicsWorld();
    if (!physicsCommon || !physicsWorld) return;

    Transform transform = Transform::identity();
    if (owner) {
        Vector3 rpPos(owner->getPosition().m_x, owner->getPosition().m_y, owner->getPosition().m_z);
        Quaternion q = eulerToQuaternion(owner->getRotation());
        transform = Transform(rpPos, q);
    }

    rigidBody = physicsWorld->createRigidBody(transform);

    createColliderFromOwner(physicsCommon, physicsWorld);


}

PhysicsComponent::~PhysicsComponent() {
    BaseComponentSystem* base = BaseComponentSystem::getInstance();
    if (base) {
        PhysicsSystem* physSys = base->getPhysicsSystem();
        if (physSys) {
            PhysicsWorld* physicsWorld = physSys->getPhysicsWorld();
            PhysicsCommon* physicsCommon = physSys->getPhysicsCommon();
            if (physicsWorld && rigidBody) {
                if (collider) {
                    rigidBody->removeCollider(collider);
                    collider = nullptr;
                }
                physicsWorld->destroyRigidBody(rigidBody);
                rigidBody = nullptr;
            }
            if (physicsCommon && collisionShape) {
                switch (shapeKind) {
                case ShapeKind::Box:
                    physicsCommon->destroyBoxShape(static_cast<BoxShape*>(collisionShape));
                    break;
                case ShapeKind::ConcaveMesh:
                    physicsCommon->destroyConcaveMeshShape(static_cast<ConcaveMeshShape*>(collisionShape));
                    if (triangleMesh) {
                        physicsCommon->destroyTriangleMesh(triangleMesh);
                        triangleMesh = nullptr;
                    }
                    break;
                default:
                    break;
                }
                collisionShape = nullptr;
                shapeKind = ShapeKind::None;
            }

            physSys->unregisterComponent(this);
        }
    }
}

void PhysicsComponent::syncOwnerToBody()
{
    if (!rigidBody || !getOwner()) return;
    Vector3D pos = getOwner()->getPosition();
    Vector3D rot = getOwner()->getRotation();
    Vector3 rpPos(pos.m_x, pos.m_y, pos.m_z);
    Quaternion q = eulerToQuaternion(rot);
    Transform t(rpPos, q);
    rigidBody->setTransform(t);
}

void PhysicsComponent::syncBodyToOwner()
{
    if (!rigidBody || !getOwner()) return;
    Transform t = rigidBody->getTransform();
    Vector3 rpPos = t.getPosition();
    Quaternion q = t.getOrientation();
    float ysqr = q.y * q.y;
    float t0 = 2.0f * (q.w * q.z + q.x * q.y);
    float t1 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    float roll = std::atan2f(t0, t1);
    float t2 = 2.0f * (q.w * q.x - q.z * q.y);
    t2 = (t2 > 1.0f) ? 1.0f : t2;
    t2 = (t2 < -1.0f) ? -1.0f : t2;
    float pitch = std::asinf(t2);
    float t3 = 2.0f * (q.w * q.y + q.z * q.x);
    float t4 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    float yaw = std::atan2f(t3, t4);

    getOwner()->setPosition(Vector3D(rpPos.x, rpPos.y, rpPos.z));
    getOwner()->setRotation(Vector3D(pitch, yaw, roll));
}

void PhysicsComponent::perform(float /*deltaTime*/) {
    if (!rigidBody || !getOwner()) {
        return;
    }

    Transform t = rigidBody->getTransform();
    Vector3 rpPos = t.getPosition();

    syncBodyToOwner();

    Vector3D ownerPos = getOwner()->getPosition();
    Transform t2 = rigidBody->getTransform();
    Vector3 rpPos2 = t2.getPosition();

}

reactphysics3d::RigidBody* PhysicsComponent::getRigidBody() const {
    return rigidBody;
}

void PhysicsComponent::setMass(float m)
{
    mass = m;
    if (!rigidBody) return;
    rigidBody->updateMassPropertiesFromColliders();
    if (mass > 0.0f) {
        rigidBody->setMass(mass);
        rigidBody->setType(BodyType::DYNAMIC);
    } else {
        rigidBody->setType(BodyType::STATIC);
    }
}

void PhysicsComponent::recreateCollider()
{
    if (auto base = BaseComponentSystem::getInstance()) {
        if (auto phys = base->getPhysicsSystem()) {
            createColliderFromOwner(phys->getPhysicsCommon(), phys->getPhysicsWorld());
        }
    }
}
