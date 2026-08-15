#pragma once
#include "ResourceManager.h"
#include <string>

class Texture;

class TextureManager : public ResourceManager
{
public:
    TextureManager();
    ~TextureManager();


    TexturePtr createTextureFromFile(const std::wstring& file_path);

protected:
    Resource* createResourceFromFileConcrete(const std::wstring& absolute_path) override;
};
