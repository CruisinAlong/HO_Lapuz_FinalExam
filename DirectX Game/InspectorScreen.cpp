#include "InspectorScreen.h"
#include "ImGui/imgui.h"
#include "AppWindow.h"
#include "MeshComponent.h"
#include "PhysicsComponent.h"
#include "BoxColliderComponent.h"
#include "Mesh.h"
#include <string>
#include "Cube.h"
#include "Plane.h"
#include "Sphere.h"
#include "Capsule.h"

InspectorScreen::InspectorScreen(AppWindow* app, int& selected) : AUIScreen("Inspector"), m_app(app), m_selected(selected) {}

void InspectorScreen::drawUI(float)
{
    ImGuiIO& io = ImGui::GetIO();
    float width = 320.0f;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - width, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(width, io.DisplaySize.y - 30), ImGuiCond_FirstUseEver);
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse);
    if (!m_app) {
        ImGui::Text("No app");
        ImGui::End();
        return;
    }

    int idx = m_selected;
    if (idx < 0 || static_cast<size_t>(idx) >= m_app->getEntityCount()) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

    auto ent = m_app->getEntity(static_cast<size_t>(idx));
    if (!ent || !ent->object) {
        ImGui::Text("Invalid entity");
        ImGui::End();
        return;
    }

    ImGui::Text("Transform");
    // Display live owner position (reflects physics updates). When edited, apply to both
    // the stored instance and the GameObject so visuals and physics stay in sync.
    Vector3D livePos = ent->object->getPosition();
    float pos[3] = { livePos.m_x, livePos.m_y, livePos.m_z };
    if (ImGui::InputFloat3("Position", pos)) {
        ent->position.m_x = pos[0]; ent->position.m_y = pos[1]; ent->position.m_z = pos[2];
        // Apply to the actual object immediately so visuals update.
        ent->object->setPosition(ent->position);
        // If a physics component is attached, move the rigid body to the owner transform
        // to ensure the physics world matches the edited transform.
        auto comps = ent->object->getComponentsOfType(AComponent::Physics);
        for (auto c : comps) {
            if (!c) continue;
            PhysicsComponent* pc = dynamic_cast<PhysicsComponent*>(c);
            if (pc && pc->getRigidBody()) {
                pc->syncOwnerToBody();
                reactphysics3d::Vector3 zero(0.0f, 0.0f, 0.0f);
                pc->getRigidBody()->setLinearVelocity(zero);
                pc->getRigidBody()->setAngularVelocity(zero);
            }
        }
    }

    // Rotation / Scale are stored per CubeInstance
    float rot[3] = { ent->rotation.m_x, ent->rotation.m_y, ent->rotation.m_z };
    if (ImGui::InputFloat3("Rotation", rot)) {
        ent->rotation.m_x = rot[0]; ent->rotation.m_y = rot[1]; ent->rotation.m_z = rot[2];
        if (ent->object) ent->object->setRotation(ent->rotation);
    }
    float scaleVal[3] = { ent->scale.m_x, ent->scale.m_y, ent->scale.m_z };
    if (ImGui::InputFloat3("Scale", scaleVal)) {
        ent->scale.m_x = scaleVal[0]; ent->scale.m_y = scaleVal[1]; ent->scale.m_z = scaleVal[2];
        if (ent->object) ent->object->setScale(ent->scale);
    }

    // Show attached components
    ImGui::Separator();
    ImGui::Text("Components:");
    if (ent->object) {
        auto comps = ent->object->getAllComponents();
        if (comps.empty()) {
            ImGui::Text("(none)");
        } else {
            // Iterate copy to allow deletion inside loop safely by breaking after change
            for (auto c : comps) {
                if (!c) continue;
                std::string name = c->getName();
                ImGui::Bullet(); ImGui::Text("%s", name.c_str());

                // Per-component remove button (unique label using pointer value)
                std::string removeLabel = std::string("Remove##") + std::to_string(reinterpret_cast<uintptr_t>(c));
                ImGui::SameLine();
                if (ImGui::SmallButton(removeLabel.c_str())) {
                    // Detach from owner and delete component (component destructor should clean up)
                    ent->object->detachComponent(c);
                    delete c;
                    // break out — UI state changed; next frame will reflect updated components
                    break;
                }

                // show type-specific details
                if (c->getType() == AComponent::MeshComp) {
                    // MeshComponent
                    MeshComponent* mc = dynamic_cast<MeshComponent*>(c);
                    if (mc) {
                        Mesh* m = mc->getMesh();
                        if (m) ImGui::Text("  Mesh: (mesh present)");
                        else ImGui::Text("  Mesh: (none)");
                        auto tex = mc->getTexture();
                        ImGui::Text("  Texture: %s", tex ? "present" : "none");
                    }
                } else if (c->getType() == AComponent::Physics) {
                    // Could be PhysicsComponent or BoxColliderComponent (both use Physics type)
                    PhysicsComponent* pc = dynamic_cast<PhysicsComponent*>(c);
                    if (pc) {
                        auto rb = pc->getRigidBody();
                        ImGui::Text("  RigidBody: %s", rb ? "present" : "none");


                        uintptr_t uid = reinterpret_cast<uintptr_t>(pc);

                        float currentMass = pc->getMass();
                        bool isDynamic = currentMass > 0.0f;
                        bool dynamicCheckbox = isDynamic;
                        std::string dynLabel = std::string("Dynamic##physics") + std::to_string(uid);
                        if (ImGui::Checkbox(dynLabel.c_str(), &dynamicCheckbox)) {
                            if (dynamicCheckbox) {
                                // Switching to dynamic: pick sensible default mass if previously zero
                                float newMass = (currentMass <= 0.0f) ? 1.0f : currentMass;
                                pc->setMass(newMass);
                                // Ensure body transform matches owner immediately
                                pc->syncOwnerToBody();
                            } else {
                                // Switching to static (collider only)
                                pc->setMass(0.0f);
                                if (pc->getRigidBody()) {
                                    pc->getRigidBody()->setLinearVelocity(reactphysics3d::Vector3(0.0f, 0.0f, 0.0f));
                                    pc->getRigidBody()->setAngularVelocity(reactphysics3d::Vector3(0.0f, 0.0f, 0.0f));
                                }
                            }
                        }
                        // If dynamic, allow editing mass
                        if (dynamicCheckbox) {
                            float massVal = pc->getMass();
                            std::string massLabel = std::string("Mass##physics") + std::to_string(uid);
                            if (ImGui::InputFloat(massLabel.c_str(), &massVal, 0.1f, 1.0f, "%.3f")) {
                                if (massVal < 0.0f) massVal = 0.0f;
                                pc->setMass(massVal);
                            }
                        }
                        // --- end new UI ---
                    }
                    BoxColliderComponent* bc = dynamic_cast<BoxColliderComponent*>(c);
                    if (bc) {
                        auto shape = bc->getShape();
                        if (shape) {
                            auto he = shape->getHalfExtents();
                            ImGui::Text("  BoxCollider: half extents = %.2f, %.2f, %.2f", he.x, he.y, he.z);
                        } else {
                            ImGui::Text("  BoxCollider: (none)");
                        }
                    }
                }
            }
        }
    }

    // Allow adding components via UI
    ImGui::Separator();
    ImGui::Text("Add Component:");
    if (ent->object) {
        ImGui::Indent(10.0f);
        if (ImGui::Button("Add Physics")) {
            PhysicsComponent* pc = new PhysicsComponent("Physics", 1.0f, ent->object);
            if (pc) ent->object->attachComponent(pc);
            if (pc) pc->syncOwnerToBody();
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Box Collider")) {
            // Choose a reasonable default half extents based on object scale
            Vector3D defaultExt(0.5f * ent->scale.m_x, 0.5f * ent->scale.m_y, 0.5f * ent->scale.m_z);
            BoxColliderComponent* bc = new BoxColliderComponent("BoxCollider", defaultExt, ent->object);
            if (bc) ent->object->attachComponent(bc);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add MeshComp")) {
            MeshComponent* mc = new MeshComponent("MeshComp", nullptr, TexturePtr(), ent->object);
            if (mc) ent->object->attachComponent(mc);
        }
        ImGui::Unindent();
    }

    // If there is no MeshComponent, still show which procedural mesh the object uses
    if (ent->object) {
        auto mcExisting = ent->object->findComponentOfType(AComponent::MeshComp);
        if (!mcExisting) {
            if (dynamic_cast<Cube*>(ent->object)) ImGui::Text("Mesh: Cube (procedural)");
            else if (dynamic_cast<Plane*>(ent->object)) ImGui::Text("Mesh: Plane (procedural)");
            else if (dynamic_cast<Sphere*>(ent->object)) ImGui::Text("Mesh: Sphere (procedural)");
            else if (dynamic_cast<Capsule*>(ent->object)) ImGui::Text("Mesh: Capsule (procedural)");
        }
    }

    if (ImGui::Button("Remove Selected")) {
        m_app->removeEntity(static_cast<size_t>(idx));
        m_selected = -1;
    }

    ImGui::End();
}
