#include "ColorPickerScreen.h"
#include "ImGui/imgui.h"
#include <algorithm>
ColorPickerScreen::ColorPickerScreen() : AUIScreen("Color Picker")
{
    m_color = ImColor(1.0f, 0.0f, 0.0f);
}

// Simple custom color picker using ImDrawList gradients and interaction
void ColorPickerScreen::drawUI(float)
{
    ImGui::Begin("Color Picker", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 picker_pos = ImGui::GetCursorScreenPos();
    static const float HUE_PICKER_WIDTH = 20.0f;
    static const ImVec2 SV_PICKER_SIZE = ImVec2(200, 200);

    // Draw SV box as a 4-corner gradient: top-left black, top-right hue, bottom-left black, bottom-right white
    float hue, sat, val;
    ImGui::ColorConvertRGBtoHSV(m_color.Value.x, m_color.Value.y, m_color.Value.z, hue, sat, val);
    ImColor hue_color = ImColor::HSV(hue, 1, 1);

    ImU32 col_tl = ImColor(0,0,0);
    ImU32 col_tr = hue_color;
    ImU32 col_bl = ImColor(0,0,0);
    ImU32 col_br = ImColor(255,255,255);

    draw_list->AddRectFilledMultiColor(picker_pos, ImVec2(picker_pos.x + SV_PICKER_SIZE.x, picker_pos.y + SV_PICKER_SIZE.y), col_tl, col_tr, col_br, col_bl);

    // Draw hue bar as 6 gradient stripes
    ImColor colors[] = {ImColor(255,0,0), ImColor(255,255,0), ImColor(0,255,0), ImColor(0,255,255), ImColor(0,0,255), ImColor(255,0,255), ImColor(255,0,0)};
    for (int i = 0; i < 6; ++i)
    {
        ImVec2 a = ImVec2(picker_pos.x + SV_PICKER_SIZE.x + 10, picker_pos.y + i * (SV_PICKER_SIZE.y / 6));
        ImVec2 b = ImVec2(picker_pos.x + SV_PICKER_SIZE.x + 10 + HUE_PICKER_WIDTH, picker_pos.y + (i + 1) * (SV_PICKER_SIZE.y / 6));
        draw_list->AddRectFilledMultiColor(a, b, colors[i], colors[i], colors[i+1], colors[i+1]);
    }

    // Interaction: saturation/value selector
    ImGui::InvisibleButton("saturation_value_selector", SV_PICKER_SIZE);
    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - picker_pos.x, ImGui::GetIO().MousePos.y - picker_pos.y);
        mouse_pos_in_canvas.x = std::min(mouse_pos_in_canvas.x, SV_PICKER_SIZE.x);
        mouse_pos_in_canvas.y = std::min(mouse_pos_in_canvas.y, SV_PICKER_SIZE.y);
        float new_v = mouse_pos_in_canvas.y / SV_PICKER_SIZE.y;
        float new_s = (SV_PICKER_SIZE.y == 0) ? 0.0f : (mouse_pos_in_canvas.x / SV_PICKER_SIZE.x) / (new_v == 0 ? 1.0f : new_v);
        new_s = std::max(0.0f, std::min(1.0f, new_s));
        new_v = std::max(0.0f, std::min(1.0f, new_v));
        val = new_v; sat = new_s;
        m_color = ImColor::HSV(hue, sat, val);
    }

    // Interaction: hue selector
    ImGui::SetCursorScreenPos(ImVec2(picker_pos.x + SV_PICKER_SIZE.x + 10, picker_pos.y));
    ImGui::InvisibleButton("hue_selector", ImVec2(HUE_PICKER_WIDTH, SV_PICKER_SIZE.y));
    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        if (ImGui::GetIO().MouseDown[0]) {
            float new_hue = (ImGui::GetIO().MousePos.y - picker_pos.y) / SV_PICKER_SIZE.y;
            new_hue = std::max(0.0f, std::min(1.0f, new_hue));
            hue = new_hue;
            m_color = ImColor::HSV(hue, sat, val);
        }
    }

    // Show editable color value below
    float rgb[3] = { m_color.Value.x, m_color.Value.y, m_color.Value.z };
    ImGui::ColorEdit3("Color", rgb);
    m_color = ImColor(rgb[0], rgb[1], rgb[2]);

    ImGui::End();
}
