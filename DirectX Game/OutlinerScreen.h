#pragma once
#include "AUIScreen.h"
class AppWindow;

class OutlinerScreen : public AUIScreen
{
public:
    OutlinerScreen(AppWindow* app, int& selected);
    void drawUI(float deltaTime) override;

private:
    AppWindow* m_app;
    int& m_selected;
};
