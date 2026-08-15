#pragma once
#include <string>

class AUIScreen
{
protected:
    typedef std::string String;

public:
    AUIScreen(String name);
    virtual ~AUIScreen();

    String getName();
    virtual void drawUI(float deltaTime) = 0;

protected:
    String name;

    friend class UIManager;
};
