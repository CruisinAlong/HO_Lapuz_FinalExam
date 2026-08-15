#include "UIManager.h"
#include "ImGui/imgui.h"
#include <cstdio>
#include "ToolbarScreen.h"
#include "OutlinerScreen.h"
#include "InspectorScreen.h"
#include "CreditsScreen.h"
#include "AppWindow.h"
#include "ColorPickerScreen.h"

UIManager::UIManager(AppWindow* app) : m_app(app)
{
    // declare and register all UI screens in desired render order
    m_screens.emplace_back(new ToolbarScreen(app));
    m_screens.emplace_back(new OutlinerScreen(app, m_selected_index));
    m_screens.emplace_back(new InspectorScreen(app, m_selected_index));
}

UIManager::~UIManager()
{
    m_screens.clear();
}

void UIManager::drawUI(float deltaTime)
{
    // Draw current screens
    for (auto& s : m_screens)
    {
        if (s) s->drawUI(deltaTime);
    }

    // Process color picker toggle request AFTER drawing (safe to modify m_screens here)
    if (m_request_toggle_color_picker) {
        bool found = false;
        // remove any existing Color Picker from active screens
        for (auto it = m_screens.begin(); it != m_screens.end(); ++it) {
            if ((*it) && (*it)->getName() == "Color Picker") {
                m_screens.erase(it);
                found = true;
                break;
            }
        }
        // also remove from pending screens if present
        if (!found) {
            for (auto it = m_pending_screens.begin(); it != m_pending_screens.end(); ++it) {
                if ((*it) && (*it)->getName() == "Color Picker") {
                    m_pending_screens.erase(it);
                    found = true;
                    break;
                }
            }
        }
        // if not present, add one (defer to pending list)
        if (!found) {
            m_pending_screens.emplace_back(new ColorPickerScreen());
            ++m_color_picker_count;
        } else {
            // removed one
            if (m_color_picker_count > 0) --m_color_picker_count;
        }
        m_request_toggle_color_picker = false;
    }

    // After drawing, move any pending screens into the main list.
    if (!m_pending_screens.empty()) {
        // Reserve once to avoid extra reallocations
        m_screens.reserve(m_screens.size() + m_pending_screens.size());
        for (auto& p : m_pending_screens) {
            m_screens.emplace_back(std::move(p));
        }
        m_pending_screens.clear();
    }

    // Draw credits window separately if requested
    // Toggle Credits screen presence based on m_show_credits
    bool credits_present = false;
    for (auto it = m_screens.begin(); it != m_screens.end(); ++it) {
        if ((*it) && (*it)->getName() == "Credits") { credits_present = true; break; }
    }

    if (m_show_credits && !credits_present) {
        // add credits screen
        m_screens.emplace_back(new CreditsScreen(m_app));
    }
    else if (!m_show_credits && credits_present) {
        // remove credits screen
        for (auto it = m_screens.begin(); it != m_screens.end(); ++it) {
            if ((*it) && (*it)->getName() == "Credits") { m_screens.erase(it); break; }
        }
    }
}

void UIManager::spawnColorPicker()
{
    m_request_toggle_color_picker = true;
}
