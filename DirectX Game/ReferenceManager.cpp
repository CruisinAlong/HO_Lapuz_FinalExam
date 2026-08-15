#include "ReferenceManager.h"

std::unordered_map<void*, int> ReferenceManager::s_map;
std::mutex ReferenceManager::s_mutex;

void ReferenceManager::acquire(void* ptr)
{
    if (!ptr) return;
    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_map.find(ptr);
    if (it == s_map.end()) s_map[ptr] = 1;
    else ++(it->second);
}

bool ReferenceManager::release(void* ptr)
{
    if (!ptr) return true;
    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_map.find(ptr);
    if (it == s_map.end()) return true;
    --(it->second);
    if (it->second <= 0) {
        s_map.erase(it);
        return true;
    }
    return false;
}
