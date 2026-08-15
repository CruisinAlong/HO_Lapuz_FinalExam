#pragma once
#include <memory>
#include <string>
#include "Resource.h"
#include "Texture.h"

class TextureManager
{
public:
    static TextureManager& instance();
    std::shared_ptr<Resource> createResourceFromFileConcrete(const std::wstring& path);
private:
    TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
};