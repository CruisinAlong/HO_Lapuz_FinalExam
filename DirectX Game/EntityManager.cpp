#include "EntityManager.h"
#include "Cube.h"
#include "MeshComponent.h"

EntityManager::~EntityManager()
{
    removeAllEntities();
}

int EntityManager::addEntity(GameObject* obj)
{
    if (!obj) return -1;
    Entity e;
    e.object = obj;
    e.position = obj->getPosition();
    e.rotation = obj->getRotation();
    e.scale = obj->getScale();
    m_entities.push_back(e);
    return static_cast<int>(m_entities.size() - 1);
}

EntityManager::Entity* EntityManager::getEntity(size_t index)
{
    if (index >= m_entities.size()) return nullptr;
    return &m_entities[index];
}

void EntityManager::removeEntity(size_t index)
{
    if (index >= m_entities.size()) return;
    if (m_entities[index].object) {
        m_entities[index].object->destroy();
        delete m_entities[index].object;
        m_entities[index].object = nullptr;
    }
    m_entities.erase(m_entities.begin() + index);
}

void EntityManager::removeAllEntities()
{
    for (size_t i = 0; i < m_entities.size(); ++i) {
        if (m_entities[i].object) {
            m_entities[i].object->destroy();
            delete m_entities[i].object;
            m_entities[i].object = nullptr;
        }
    }
    m_entities.clear();
    m_pendingRemovals.clear();
}

void EntityManager::processPendingRemovals()
{
    if (m_pendingRemovals.empty()) return;
    std::sort(m_pendingRemovals.begin(), m_pendingRemovals.end());
    m_pendingRemovals.erase(std::unique(m_pendingRemovals.begin(), m_pendingRemovals.end()), m_pendingRemovals.end());
    for (auto it = m_pendingRemovals.rbegin(); it != m_pendingRemovals.rend(); ++it) {
        size_t idx = *it;
        if (idx >= m_entities.size()) continue;
        if (m_entities[idx].object) {
            m_entities[idx].object->destroy();
            delete m_entities[idx].object;
            m_entities[idx].object = nullptr;
        }
        m_entities.erase(m_entities.begin() + idx);
    }
    m_pendingRemovals.clear();
}
