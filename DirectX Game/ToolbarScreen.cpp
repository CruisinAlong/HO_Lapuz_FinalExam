#include "ToolbarScreen.h"
#include "ImGui/imgui.h"
#include "AppWindow.h"
#include "UIManager.h"
#include "UIConfig.h"

ToolbarScreen::ToolbarScreen(AppWindow* app) : AUIScreen("Toolbar"), m_app(app) {}

void ToolbarScreen::drawUI(float)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, TOOLBAR_HEIGHT), ImGuiCond_Always);
    ImGui::Begin("Toolbar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            bool canSave = (m_app && !m_app->isSimulationRunning());
            if (ImGui::MenuItem("Save Level...", NULL, false, canSave)) {
                if (m_app) m_app->saveLevelAs();
            }
            if (!canSave && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Pause simulation to save the current editor state.");
            }

            if (ImGui::MenuItem("Load Level...")) {
                if (m_app) m_app->loadLevelFromDialog();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("About")) {
            if (ImGui::MenuItem("Credits")) {
                if (m_app && m_app->getUIManager()) {
                    // toggle credits window
                    m_app->getUIManager()->setShowCredits(!m_app->getUIManager()->isShowCredits());
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGui::Button("Add Basic Cube")) { if (m_app) m_app->addCube(); }
    ImGui::SameLine();
    if (ImGui::Button("Add Textured Cube")) { if (m_app) m_app->addTexturedCube(); }
    ImGui::SameLine();
    if (ImGui::Button("Add Sphere")) { if (m_app) m_app->addSphere(); }
    ImGui::SameLine();
    if (ImGui::Button("Add Physics Cube")) {
        if (m_app) {
            // spawn 20 physics cubes at the same location for collision testing
            for (int i = 0; i < 20; ++i) m_app->addPhysicsCube();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Physics Plane")) { if (m_app) m_app->addPhysicsPlane(); }
    ImGui::SameLine();
    if (ImGui::Button("Remove All Cubes")) { if (m_app) m_app->removeAllCubes(); }
    ImGui::SameLine();
    // Play/Pause simulation toggle
    if (m_app) {
        const char* label = m_app->isSimulationRunning() ? "Pause" : "Play";
        if (ImGui::Button(label)) { m_app->setSimulationRunning(!m_app->isSimulationRunning()); }
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Capsule")) { if (m_app) m_app->addCapsule(2.0f); }
    ImGui::SameLine();
    if (ImGui::Button("Help")) {}
    ImGui::End();
}
