#include "SceneSerializer.h"
#include "SceneEditor.h"
#include "SceneBuilder.h"
#include "PhysicsComponent.h"
#include "BoxColliderComponent.h"
#include "MeshComponent.h"
#include "GraphicsEngine.h"
#include <sstream>

bool SceneSerializer::saveInstances(const std::wstring& filename, const std::vector<ObjectInstance>& instances)
{
    std::vector<SceneEditor::SerializedObject> list;
    list.reserve(instances.size());
    for (const auto& inst : instances) {
        if (!inst.object) continue;
        SceneEditor::SerializedObject so;
        if (dynamic_cast<Capsule*>(inst.object)) so.type = "Capsule";
        else if (dynamic_cast<Sphere*>(inst.object)) so.type = "Sphere";
        else if (dynamic_cast<Plane*>(inst.object)) so.type = "Plane";
        else so.type = "Cube";
        so.position = inst.position;
        so.rotation = inst.rotation;
        so.scale = inst.scale;

        auto comps = inst.object->getAllComponents();
        for (auto c : comps) {
            if (!c) continue;
            SceneEditor::ComponentRecord cr;
            if (auto pc = dynamic_cast<PhysicsComponent*>(c)) {
                cr.type = "Physics";
                cr.properties.emplace_back("mass", std::to_string(pc->getMass()));
            } else if (auto bc = dynamic_cast<BoxColliderComponent*>(c)) {
                cr.type = "BoxCollider";
                auto shape = bc->getShape();
                if (shape) {
                    auto he = shape->getHalfExtents();
                    std::ostringstream oss;
                    oss << "[" << he.x << "," << he.y << "," << he.z << "]";
                    cr.properties.emplace_back("halfExtents", oss.str());
                }
            } else if (auto mc = dynamic_cast<MeshComponent*>(c)) {
                cr.type = "MeshComponent";
                cr.properties.emplace_back("hasMesh", "1");
            }
            if (!cr.type.empty()) so.components.push_back(cr);
        }
        list.push_back(so);
    }
    return SceneEditor::saveLevel(filename, list);
}

bool SceneSerializer::loadInstances(const std::wstring& filename, std::vector<ObjectInstance>& outInstances, SceneBuilder* builder)
{
    if (!outInstances.empty()) {
        if (GraphicsEngine::getInstance()) {
            auto rs = GraphicsEngine::getInstance()->getRenderSystem();
            if (rs) {
                auto ctx = rs->getImmediateDeviceContext().get();
                if (ctx) {
                    ctx->resetStateBindings();
                }
            }
        }

        for (size_t i = 0; i < outInstances.size(); ++i) {
            if (outInstances[i].object) {
                outInstances[i].object->destroy();
                delete outInstances[i].object;
                outInstances[i].object = nullptr;
            }
        }
        outInstances.clear();
    }

    std::vector<SceneEditor::SerializedObject> list;
    if (!SceneEditor::loadLevel(filename, list)) return false;

    SceneBuilder localBuilder;
    SceneBuilder* usedBuilder = builder ? builder : &localBuilder;

    for (const auto& so : list) {
        ObjectInstance inst;
        if (so.type == "Capsule") {
            inst = usedBuilder->createCapsule(so.scale.m_y > 0.0f ? so.scale.m_y : 1.0f, so.position);
        } else if (so.type == "Sphere") {
            inst.object = new Sphere();
            if (inst.object && static_cast<Sphere*>(inst.object)->create()) {
                inst.position = so.position;
                inst.rotation = so.rotation;
                inst.scale = so.scale;
                inst.object->setPosition(inst.position);
                inst.object->setRotation(inst.rotation);
                inst.object->setScale(inst.scale);
            } else {
                if (inst.object) { inst.object->destroy(); delete inst.object; inst.object = nullptr; }
            }
        } else if (so.type == "Plane") {
            inst = usedBuilder->createPhysicsPlane(so.position, so.scale);
        } else if (so.type == "PhysicsCube") {
            inst = usedBuilder->createPhysicsCube(1.0f, so.position, so.rotation, so.scale, TexturePtr());
        } else {
            inst = usedBuilder->createCube(so.position, so.rotation, so.scale, TexturePtr());
        }

        if (!inst.object) continue;

        for (const auto& cr : so.components) {
            if (cr.type == "Physics") {
                float mass = 0.0f;
                for (const auto& kv : cr.properties) {
                    if (kv.first == "mass") {
                        try { mass = std::stof(kv.second); } catch (...) { mass = 0.0f; }
                        break;
                    }
                }
                AComponent* existing = inst.object->findComponentOfType(AComponent::Physics);
                if (existing) {
                    PhysicsComponent* epc = dynamic_cast<PhysicsComponent*>(existing);
                    if (epc) {
                        epc->setMass(mass);
                        epc->recreateCollider();
                        epc->syncOwnerToBody();
                        if (epc->getRigidBody()) {
                            reactphysics3d::Vector3 zero(0.0f,0.0f,0.0f);
                            epc->getRigidBody()->setLinearVelocity(zero);
                            epc->getRigidBody()->setAngularVelocity(zero);
                        }
                    }
                } else {
                    PhysicsComponent* pc = new PhysicsComponent("PhysicsLoaded", mass, inst.object);
                    if (pc) {
                        inst.object->attachComponent(pc);
                        pc->syncOwnerToBody();
                        if (pc->getRigidBody()) {
                            reactphysics3d::Vector3 zero(0.0f,0.0f,0.0f);
                            pc->getRigidBody()->setLinearVelocity(zero);
                            pc->getRigidBody()->setAngularVelocity(zero);
                        }
                    }
                }
            } else if (cr.type == "BoxCollider") {
                Vector3D he(0.5f,0.5f,0.5f);
                for (const auto& kv : cr.properties) {
                    if (kv.first == "halfExtents") {
                        std::string v = kv.second;
                        size_t a = v.find('[');
                        size_t b = v.find(']');
                        if (a != std::string::npos && b != std::string::npos && b > a) {
                            std::string inside = v.substr(a+1, b-a-1);
                            std::istringstream iss(inside);
                            float x=0,y=0,z=0; char comma;
                            iss >> x >> comma >> y >> comma >> z;
                            he.m_x = x; he.m_y = y; he.m_z = z;
                        }
                        break;
                    }
                }
                BoxColliderComponent* bc = new BoxColliderComponent("BoxColliderLoaded", he, inst.object);
                if (bc) inst.object->attachComponent(bc);
            } else if (cr.type == "MeshComponent") {
                MeshComponent* mc = new MeshComponent("MeshCompLoaded", nullptr, TexturePtr(), inst.object);
                if (mc) inst.object->attachComponent(mc);
            }
        }

        outInstances.push_back(inst);
    }

    return true;
}
