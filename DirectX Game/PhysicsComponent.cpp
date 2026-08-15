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

#include "PhysicsPlane.h" // detect plane owners

using namespace reactphysics3d;

static Quaternion eulerToQuaternion(const Vector3D& euler)
{
    // Engine uses rotation order Y * X * Z
    float cx = std::cosf(euler.m_x * 0.5f);
    float sx = std::sinf(euler.m_x * 0.5f);
    float cy = std::cosf(euler.m_y * 0.5f);
    float sy = std::sinf(euler.m_y * 0.5f);
    float cz = std::cosf(euler.m_z * 0.5f);
    float sz = std::sinf(euler.m_z * 0.5f);

    Quaternion qx(sx, 0, 0, cx);
    Quaternion qy(0, sy, 0, cy);
    Quaternion qz(0, 0, sz, cz);
    // q = qy * qx * qz
    return qy * qx * qz;
}

// Helper: create collider from owner scale/transform
void PhysicsComponent::createColliderFromOwner(PhysicsCommon* physicsCommon, PhysicsWorld* physicsWorld)
{
    if (!physicsCommon || !physicsWorld || !getOwner() || !rigidBody) return;

    // Remove existing collider if present
    if (collider) {
        rigidBody->removeCollider(collider);
        collider = nullptr;
    }

    // Destroy previous collision shape according to the recorded shapeKind
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

    // If owner is a PhysicsPlane, create a thin mesh (two triangles) matching plane geometry
    if (dynamic_cast<PhysicsPlane*>(getOwner())) {
        // Create a small triangle mesh (unit plane, halfExtent = 0.5) and scale to owner's scale
        const float halfExtent = 0.5f;
        // 4 vertices (x,y,z) - tightly packed floats
        float vertices[4 * 3] = {
            -halfExtent, 0.0f, -halfExtent,  // v0
            -halfExtent, 0.0f,  halfExtent,  // v1
             halfExtent, 0.0f, -halfExtent,  // v2
             halfExtent, 0.0f,  halfExtent   // v3
        };
        // Two triangles (indices)
        uint32_t indices[2 * 3] = {
            0, 1, 2,
            1, 3, 2
        };

        // Build a TriangleVertexArray (float vertices, integer indices)
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
            // Use owner's scale as scaling for the concave shape so the visual and collider match
            Vector3D s = getOwner()->getScale();
            Vector3 scaling(s.m_x, s.m_y, s.m_z);
            ConcaveMeshShape* concave = physicsCommon->createConcaveMeshShape(triMesh, scaling);
            if (concave) {
                // store pointers so they can be destroyed in destructor
                collisionShape = concave;
                triangleMesh = triMesh;
                shapeKind = ShapeKind::ConcaveMesh;
                collider = rigidBody->addCollider(collisionShape, Transform::identity());
                // Ensure rigid body is static for plane
                rigidBody->setType(BodyType::STATIC);
                // For static bodies skip mass update
                return;
            } else {
                // Fallback: destroy triMesh if concave creation failed
                physicsCommon->destroyTriangleMesh(triMesh);
            }
        }
        // If mesh creation failed, fall back to box below
    }

    // Default: create a box collider using owner scale
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

    // Update mass/inertia for dynamic bodies only
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

    // Ensure each physics component registers with a unique name so multiple components
    // of the same conceptual type can be tracked individually by PhysicsSystem.
    // Append the pointer value to the name to guarantee uniqueness.
    {
        std::string uniqueSuffix = "_" + std::to_string(reinterpret_cast<uintptr_t>(this));
        this->name += uniqueSuffix;
    }

    physSys->registerComponent(this);


    PhysicsCommon* physicsCommon = physSys->getPhysicsCommon();
    PhysicsWorld* physicsWorld = physSys->getPhysicsWorld();
    if (!physicsCommon || !physicsWorld) return;

    // Build initial rigid-body transform from owner's position/rotation
    Transform transform = Transform::identity();
    if (owner) {
        Vector3 rpPos(owner->getPosition().m_x, owner->getPosition().m_y, owner->getPosition().m_z);
        Quaternion q = eulerToQuaternion(owner->getRotation());
        transform = Transform(rpPos, q);
    }

    rigidBody = physicsWorld->createRigidBody(transform);

    // create collider/shape based on owner's current scale via helper
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
            // destroy shape(s) created earlier (box / triangle mesh / concave) if stored
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
    // Convert quaternion to Euler. Use safe conversion for Y*X*Z order.
    // We'll extract yaw (Y), pitch (X), roll (Z) in the assumed order.
    float ysqr = q.y * q.y;
    // roll (Z)
    float t0 = 2.0f * (q.w * q.z + q.x * q.y);
    float t1 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    float roll = std::atan2f(t0, t1);
    // pitch (X)
    float t2 = 2.0f * (q.w * q.x - q.z * q.y);
    t2 = (t2 > 1.0f) ? 1.0f : t2;
    t2 = (t2 < -1.0f) ? -1.0f : t2;
    float pitch = std::asinf(t2);
    // yaw (Y)
    float t3 = 2.0f * (q.w * q.y + q.z * q.x);
    float t4 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    float yaw = std::atan2f(t3, t4);

    getOwner()->setPosition(Vector3D(rpPos.x, rpPos.y, rpPos.z));
    getOwner()->setRotation(Vector3D(pitch, yaw, roll));
}

void PhysicsComponent::perform(float /*deltaTime*/) {
    // Log rigid body transform and owner transform before and after syncing so we can see whether
    // physics moved the body and whether the owner received the updated transform.
    if (!rigidBody || !getOwner()) {
        return;
    }

    // Read rigid body transform before sync
    Transform t = rigidBody->getTransform();
    Vector3 rpPos = t.getPosition();

    // Perform the usual sync (copy body transform into owner)
    syncBodyToOwner();

    // Log owner transform after sync
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
    // Recompute mass properties from colliders then apply mass/type
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
