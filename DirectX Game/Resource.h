#pragma once
#include <string>

class Resource
{
public:
    Resource(const std::wstring& full_path);
    virtual ~Resource();

    const std::wstring& getFullPath() const { return m_full_path; }

protected:
    std::wstring m_full_path;
};
