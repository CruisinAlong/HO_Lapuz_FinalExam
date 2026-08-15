#pragma once
#include "AUIScreen.h"
#include <d3d11.h>

class AppWindow;

class CreditsScreen : public AUIScreen
{
public:
    CreditsScreen(AppWindow* app);
    ~CreditsScreen();
    void drawUI(float deltaTime) override;

private:
    bool loadImage();
    void releaseImage();

    AppWindow* m_app;
    ID3D11ShaderResourceView* m_srv;
    int m_width;
    int m_height;
    char m_buf[256];
};
