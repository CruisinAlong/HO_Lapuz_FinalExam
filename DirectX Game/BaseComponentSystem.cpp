#include "BaseComponentSystem.h"
#include "AComponent.h"
#include "PhysicsSystem.h"
#include <algorithm>
#include <exception>

BaseComponentSystem* BaseComponentSystem::sharedInstance = nullptr;

BaseComponentSystem* BaseComponentSystem::getInstance() {
    return sharedInstance;
}

BaseComponentSystem* BaseComponentSystem::get() {
    return sharedInstance;
}

void BaseComponentSystem::create() {
    if (!sharedInstance) {
        try {
            sharedInstance = new BaseComponentSystem();
        }
        catch (const std::exception& /*ex*/) {
            if (sharedInstance) { delete sharedInstance; sharedInstance = nullptr; }
        }
        catch (...) {
            if (sharedInstance) { delete sharedInstance; sharedInstance = nullptr; }
        }
    }
}

void BaseComponentSystem::destroy() {
    if (sharedInstance) {
        delete sharedInstance;
        sharedInstance = nullptr;
    }
}

bool BaseComponentSystem::init() {
    // ctor handled initialization; keep for parity with GraphicsEngine if needed
    return true;
}

bool BaseComponentSystem::release() {
    // nothing special for now
    return true;
}

BaseComponentSystem::BaseComponentSystem() {
    // Create and own a PhysicsSystem so PhysicsComponents can register and the world will step.
    physicsSystem = new PhysicsSystem();
}

BaseComponentSystem::~BaseComponentSystem() {
    // Destroy physics system if present
    if (physicsSystem) {
        delete physicsSystem;
        physicsSystem = nullptr;
    }
    // Note: AComponent lifetime is still owned by their GameObject owners
    m_components.clear();
}

void BaseComponentSystem::registerComponent(AComponent* comp) {
    if (!comp) return;
    m_components.push_back(comp);
}

void BaseComponentSystem::unregisterComponent(AComponent* comp) {
    auto it = std::find(m_components.begin(), m_components.end(), comp);
    if (it != m_components.end()) m_components.erase(it);
}

const std::vector<AComponent*>& BaseComponentSystem::getComponents() const {
    return m_components;
}

PhysicsSystem* BaseComponentSystem::getPhysicsSystem() {
    return physicsSystem;
}
