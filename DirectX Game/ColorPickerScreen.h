#pragma once
#include "AUIScreen.h"
#include "ImGui/imgui.h"

class ColorPickerScreen : public AUIScreen
{
public:
    ColorPickerScreen();
    void drawUI(float deltaTime) override;

private:
    ImColor m_color;
};
