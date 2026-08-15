#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

class PhysicsManager {
public:
    PhysicsManager() = default;
    ~PhysicsManager() = default;

    void step(float deltaTime);
};

#endif 
