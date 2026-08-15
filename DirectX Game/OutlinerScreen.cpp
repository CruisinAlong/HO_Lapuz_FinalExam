#include "OutlinerScreen.h"
#include "ImGui/imgui.h"
#include "AppWindow.h"
#include "UIConfig.h"

OutlinerScreen::OutlinerScreen(AppWindow* app, int& selected) : AUIScreen("Outliner"), m_app(app), m_selected(selected) {}

void OutlinerScreen::drawUI(float)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, TOOLBAR_HEIGHT), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(220, io.DisplaySize.y - TOOLBAR_HEIGHT), ImGuiCond_FirstUseEver);
    ImGui::Begin("World Outliner", nullptr, ImGuiWindowFlags_NoCollapse);
    // List entities from AppWindow
    if (m_app) {
        size_t count = m_app->getEntityCount();
        for (size_t i = 0; i < count; ++i)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "Object %zu", i);
            // Visibility checkbox per-entity (allows multiple visible at once)
            auto ent = m_app->getEntity(i);
            if (ent) {
                char visLabel[64];
                snprintf(visLabel, sizeof(visLabel), "##vis%zu", i);
                bool vis = ent->visible;
                if (ImGui::Checkbox(visLabel, &vis)) {
                    ent->visible = vis;
                }
                ImGui::SameLine();
            }

            bool sel = (static_cast<int>(i) == m_selected);
            if (ImGui::Selectable(buf, sel)) {
                m_selected = static_cast<int>(i);
            }
        }
    }
    ImGui::End();
}
