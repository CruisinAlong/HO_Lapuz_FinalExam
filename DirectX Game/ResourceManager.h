#pragma once
#include "Prerequisites.h"
#include <string>
#include <unordered_map>
#include <memory>

class ResourceManager
{
public:
    ResourceManager();
    virtual ~ResourceManager();


    Resource* createResourceFromFile(const std::wstring& file_path);

    Resource* getResourceByPath(const std::wstring& file_path) const;

protected:
    virtual Resource* createResourceFromFileConcrete(const std::wstring& absolute_path) = 0;

    std::unordered_map<std::wstring, ResourcePtr> m_resources;
};
