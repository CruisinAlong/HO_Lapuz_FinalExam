#include "TextureManager.h"
#include "Texture.h"
#include <Windows.h>
#include <stdexcept>

TextureManager::TextureManager() {}
TextureManager::~TextureManager() {}

Resource* TextureManager::createResourceFromFileConcrete(const std::wstring& absolute_path)
{

    try {
        Texture* tex = new Texture(absolute_path);
        return tex;
    }
    catch (const std::exception&)
    {
        return nullptr;
    }
}

TexturePtr TextureManager::createTextureFromFile(const std::wstring& file_path)
{
    if (file_path.empty()) return TexturePtr();

    wchar_t buf[MAX_PATH];
    DWORD len = ::GetFullPathNameW(file_path.c_str(), MAX_PATH, buf, nullptr);
    std::wstring key;
    if (len == 0) key = file_path; else key.assign(buf, buf + len);


    Resource* r = createResourceFromFile(key);
    if (!r) return TexturePtr();

    auto it = m_resources.find(key);
    if (it == m_resources.end()) return TexturePtr();

    return std::static_pointer_cast<Texture>(it->second);
}
