#pragma once
#include "AUIScreen.h"
class AppWindow;

class ToolbarScreen : public AUIScreen
{
public:
    ToolbarScreen(AppWindow* app);
    void drawUI(float deltaTime) override;

private:
    AppWindow* m_app;
};
