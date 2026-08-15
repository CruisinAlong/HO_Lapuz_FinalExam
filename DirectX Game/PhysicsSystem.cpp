#pragma message("Including RP3D from: " __FILE__)
#include <reactphysics3d/reactphysics3d.h>

#include "PhysicsSystem.h"
#include "PhysicsComponent.h"

#include <algorithm>
#include <iostream>
#include <cassert>

using namespace reactphysics3d;

PhysicsSystem::PhysicsSystem() {
    physicsCommon = new PhysicsCommon();
    physicsWorld = physicsCommon->createPhysicsWorld();


    if (physicsWorld) {
        physicsWorld->setGravity(reactphysics3d::Vector3(0.0f, -9.81f, 0.0f));
    }
}

PhysicsSystem::~PhysicsSystem() {
    if (physicsCommon) {
        if (physicsWorld) {
            physicsCommon->destroyPhysicsWorld(physicsWorld);
            physicsWorld = nullptr;
        }
        delete physicsCommon;
        physicsCommon = nullptr;
    }
    componentList.clear();
    componentTable.clear();
}

void PhysicsSystem::registerComponent(PhysicsComponent* component) {
    if (!component) return;
    const String& name = component->getName();
    if (componentTable.find(name) == componentTable.end()) {
        componentTable[name] = component;
        componentList.push_back(component);
    }
}

void PhysicsSystem::unregisterComponent(PhysicsComponent* component) {
    if (!component) return;
    const String& name = component->getName();
    auto it = componentTable.find(name);
    if (it != componentTable.end()) componentTable.erase(it);

    auto vit = std::find(componentList.begin(), componentList.end(), component);
    if (vit != componentList.end()) componentList.erase(vit);
}

void PhysicsSystem::unregisterComponentByName(const String& name) {
    auto it = componentTable.find(name);
    if (it != componentTable.end()) {
        PhysicsComponent* comp = it->second;
        componentTable.erase(it);
        auto vit = std::find(componentList.begin(), componentList.end(), comp);
        if (vit != componentList.end()) componentList.erase(vit);
    }
}

PhysicsComponent* PhysicsSystem::findComponentByName(const String& name) {
    auto it = componentTable.find(name);
    return (it != componentTable.end()) ? it->second : nullptr;
}

PhysicsSystem::ComponentList PhysicsSystem::getAllComponents() const {
    return componentList; // copy - caller can iterate safely
}

void PhysicsSystem::updateAllComponents(float deltaTime) {
    if (!physicsWorld) return;



    const float clampedDelta = std::max(deltaTime, 1e-6f);

    // Call into physics world with safe, clamped timestep.
    physicsWorld->update(clampedDelta);

    // Sync each registered component from the physics world back to the owner
    for (auto comp : componentList) {
        if (comp) comp->perform(clampedDelta);
    }
}

PhysicsWorld* PhysicsSystem::getPhysicsWorld() {
    return physicsWorld;
}

PhysicsCommon* PhysicsSystem::getPhysicsCommon() {
    return physicsCommon;
}
