#pragma once

#include <string>
#include <unordered_map>
#include <vector>

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

    void updateAllComponents(float deltaTime);

    reactphysics3d::PhysicsWorld* getPhysicsWorld();
    reactphysics3d::PhysicsCommon* getPhysicsCommon();

private:
    ComponentTable componentTable;
    ComponentList componentList;

    reactphysics3d::PhysicsCommon* physicsCommon = nullptr;
    reactphysics3d::PhysicsWorld* physicsWorld = nullptr;
};
