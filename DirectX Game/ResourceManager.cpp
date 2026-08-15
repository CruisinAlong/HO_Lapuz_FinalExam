#include "ResourceManager.h"

// Use Win32 API to compute absolute path (portable for this project/toolset)
ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    m_resources.clear();
}

Resource* ResourceManager::createResourceFromFile(const std::wstring& file_path)
{
    if (file_path.empty()) return nullptr;
    std::wstring key;
    wchar_t buf[MAX_PATH];
    DWORD len = ::GetFullPathNameW(file_path.c_str(), MAX_PATH, buf, nullptr);
    if (len == 0) key = file_path; else key.assign(buf, buf + len);

    auto it = m_resources.find(key);
    if (it != m_resources.end()) {
        return it->second.get();
    }

    // Not found: create concrete resource
    Resource* raw = createResourceFromFileConcrete(key);
    if (!raw) return nullptr;

    ResourcePtr sp(raw);
    m_resources.emplace(key, sp);
    return raw;
}

Resource* ResourceManager::getResourceByPath(const std::wstring& file_path) const
{
    if (file_path.empty()) return nullptr;
    std::wstring key;
    wchar_t buf[MAX_PATH];
    DWORD len = ::GetFullPathNameW(file_path.c_str(), MAX_PATH, buf, nullptr);
    if (len == 0) key = file_path; else key.assign(buf, buf + len);

    auto it = m_resources.find(key);
    if (it == m_resources.end()) return nullptr;
    return it->second.get();
}
