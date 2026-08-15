#pragma once
#include "CorePrereqs.h"
#include "GameObject.h"

// Lightweight entity container separate from AppWindow; stores simple instance data
class EntityManager {
public:
    struct Entity {
        GameObject* object = nullptr;
        Vector3D position = Vector3D(0.0f,0.0f,0.0f);
        Vector3D rotation = Vector3D(0.0f,0.0f,0.0f);
        Vector3D scale = Vector3D(1.0f,1.0f,1.0f);
        bool visible = true;
        uint32_t componentMask = 0;
    };

    EntityManager() {}
    ~EntityManager();

    int addEntity(GameObject* obj);
    size_t getEntityCount() const { return m_entities.size(); }
    Entity* getEntity(size_t index);

    void removeEntity(size_t index);
    void removeAllEntities();

    // iterate pending updates (placeholder)
    void processPendingRemovals();

private:
    std::vector<Entity> m_entities;
    std::vector<size_t> m_pendingRemovals;
};
