#pragma once
#include <unordered_map>
#include <mutex>

class ReferenceManager
{
public:
    // increment reference count for pointer
    static void acquire(void* ptr);
    // decrement reference count for pointer; return true if count reached zero
    static bool release(void* ptr);
private:
    static std::unordered_map<void*, int> s_map;
    static std::mutex s_mutex;
};
