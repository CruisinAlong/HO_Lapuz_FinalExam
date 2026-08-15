#pragma once
#include <string>
#include <vector>
#include "AppWindow.h" // for ObjectInstance

class SceneBuilder;

class SceneSerializer {
public:
    // Save a vector of ObjectInstance to filename
    static bool saveInstances(const std::wstring& filename, const std::vector<ObjectInstance>& instances);
    // Load and instantiate objects described in filename into the provided vector.
    // If 'builder' is provided, it will be used to create objects (prevents creating a second builder
    // and keeps creation consistent with the AppWindow-owned SceneBuilder). If nullptr, a local builder
    // will be used. The method will safely destroy any existing objects in outInstances before loading.
    static bool loadInstances(const std::wstring& filename, std::vector<ObjectInstance>& outInstances, SceneBuilder* builder = nullptr);
};
