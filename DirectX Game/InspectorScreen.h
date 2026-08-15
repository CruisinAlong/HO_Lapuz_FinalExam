#pragma once
#include "AUIScreen.h"
class AppWindow;

class InspectorScreen : public AUIScreen
{
public:
    InspectorScreen(AppWindow* app, int& selected);
    void drawUI(float deltaTime) override;

private:
    AppWindow* m_app;
    int& m_selected;
};
