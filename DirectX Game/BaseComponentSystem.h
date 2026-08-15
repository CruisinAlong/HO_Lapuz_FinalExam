#pragma once
#include <vector>
#include <memory>

class AComponent;
class PhysicsSystem;

class BaseComponentSystem {
public:

    static BaseComponentSystem* getInstance();
    static BaseComponentSystem* get(); 
    static void create();
    static void destroy();


    bool init();
    bool release();


    void registerComponent(AComponent* comp);
    void unregisterComponent(AComponent* comp);


    const std::vector<AComponent*>& getComponents() const;

    PhysicsSystem* getPhysicsSystem();

private:
    BaseComponentSystem();
    ~BaseComponentSystem();
    BaseComponentSystem(const BaseComponentSystem&) = delete;
    BaseComponentSystem& operator=(const BaseComponentSystem&) = delete;

    static BaseComponentSystem* sharedInstance;
    std::vector<AComponent*> m_components;
    PhysicsSystem* physicsSystem = nullptr;
};
