#pragma once
#include <unordered_map>
#include <mutex>

class ReferenceManager
{
public:
    static void acquire(void* ptr);
    static bool release(void* ptr);
private:
    static std::unordered_map<void*, int> s_map;
    static std::mutex s_mutex;
};
