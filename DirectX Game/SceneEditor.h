#pragma once
#include <string>
#include <vector>
#include "Vector3D.h"

class SceneEditor
{
public:
    struct ComponentRecord {
        std::string type; 
        std::vector<std::pair<std::string, std::string>> properties;
    };

    struct SerializedObject {
        std::string type; 
        Vector3D position;
        Vector3D rotation;
        Vector3D scale;
        std::vector<ComponentRecord> components; 
    };

    static bool saveLevel(const std::wstring& filename, const std::vector<SerializedObject>& objects);


    static bool loadLevel(const std::wstring& filename, std::vector<SerializedObject>& outObjects);
};
