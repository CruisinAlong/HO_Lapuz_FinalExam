#include "PhysicsManager.h"
#include "BaseComponentSystem.h"
#include "PhysicsSystem.h"

void PhysicsManager::step(float deltaTime)
{
    if (auto base = BaseComponentSystem::getInstance()) {
        if (auto phys = base->getPhysicsSystem()) {
            phys->updateAllComponents(deltaTime);
        }
    }
}
