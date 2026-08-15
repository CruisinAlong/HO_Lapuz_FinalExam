#pragma once
#include "AUIScreen.h"
#include <vector>
#include <memory>

class AppWindow;

class UIManager
{
public:
    UIManager(AppWindow* app);
    ~UIManager();

    void drawUI(float deltaTime);

    void setShowCredits(bool show) { m_show_credits = show; }
    bool isShowCredits() const { return m_show_credits; }

    void spawnColorPicker();

private:
    std::vector<std::unique_ptr<AUIScreen>> m_screens;
    std::vector<std::unique_ptr<AUIScreen>> m_pending_screens;
    AppWindow* m_app;
    int m_selected_index = -1;
    bool m_show_credits = false;
    int m_color_picker_count = 0;

    bool m_request_toggle_color_picker = false;
};
