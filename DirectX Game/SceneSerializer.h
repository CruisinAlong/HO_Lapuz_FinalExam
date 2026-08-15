#pragma once
#include <string>
#include <vector>
#include "AppWindow.h"

class SceneBuilder;

class SceneSerializer {
public:
    static bool saveInstances(const std::wstring& filename, const std::vector<ObjectInstance>& instances);

    static bool loadInstances(const std::wstring& filename, std::vector<ObjectInstance>& outInstances, SceneBuilder* builder = nullptr);
};
