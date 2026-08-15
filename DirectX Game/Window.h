#pragma once
#include <Windows.h>
#define NOMINMAX
class Window
{
public:
    Window();          

    bool init();
    bool broadcast();
    bool release();
    bool isRun();

    RECT getClientWindowRect();
    void setHWND(HWND hwnd);

    virtual void onCreate()=0;
    virtual void onUpdate()=0 ;
    virtual void onDestroy();
    virtual void onFocus() {}
    virtual void onKillFocus() {}

    ~Window();
protected:
    HWND m_hwnd;
    bool m_isRun;
    bool m_initialized = false;
};

