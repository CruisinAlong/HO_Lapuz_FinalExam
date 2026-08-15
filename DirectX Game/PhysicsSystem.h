#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations for ReactPhysics3D types to avoid leaking RP3D headers into other TUs.
namespace reactphysics3d {
    class PhysicsCommon;
    class PhysicsWorld;
}

class PhysicsComponent;

class PhysicsSystem {
public:
    typedef std::string String;
    typedef std::unordered_map<String, PhysicsComponent*> ComponentTable;
    typedef std::vector<PhysicsComponent*> ComponentList;

    PhysicsSystem();
    ~PhysicsSystem();

    void registerComponent(PhysicsComponent* component);
    void unregisterComponent(PhysicsComponent* component);
    void unregisterComponentByName(const String& name);
    PhysicsComponent* findComponentByName(const String& name);
    ComponentList getAllComponents() const;

    // Step world and update components
    void updateAllComponents(float deltaTime);

    // Accessors for RP3D objects (forward-declared types)
    reactphysics3d::PhysicsWorld* getPhysicsWorld();
    reactphysics3d::PhysicsCommon* getPhysicsCommon();

private:
    ComponentTable componentTable;
    ComponentList componentList;

    reactphysics3d::PhysicsCommon* physicsCommon = nullptr;
    reactphysics3d::PhysicsWorld* physicsWorld = nullptr;
};
