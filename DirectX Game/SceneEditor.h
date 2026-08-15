#pragma once
#include <string>
#include <vector>
#include "Vector3D.h"

class SceneEditor
{
public:
    struct ComponentRecord {
        std::string type; // component type identifier, e.g. "Physics", "BoxCollider"
        // simple key/value properties; value stored as raw JSON-ish string (numbers or arrays or quoted strings)
        std::vector<std::pair<std::string, std::string>> properties;
    };

    struct SerializedObject {
        std::string type; // object type identifier, e.g. "Capsule"
        Vector3D position;
        Vector3D rotation;
        Vector3D scale;
        std::vector<ComponentRecord> components; // optional list of attached components + properties
    };

    // Save the given objects to a JSON .level file. Returns true on success.
    static bool saveLevel(const std::wstring& filename, const std::vector<SerializedObject>& objects);

    // Load objects from a JSON .level file produced by saveLevel. Returns true on success.
    // The loaded objects contain type, transform and optional component data; instantiation must be handled by the caller.
    static bool loadLevel(const std::wstring& filename, std::vector<SerializedObject>& outObjects);
};
