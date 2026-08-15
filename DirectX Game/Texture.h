#pragma once
#include "Resource.h"
#include <d3d11.h>
#include <wrl.h>
#include <string>

class Texture : public Resource
{
public:
    Texture(const std::wstring& path);
    ~Texture();

    ID3D11ShaderResourceView* getSRV() const { return m_srv; }
    ID3D11Resource* getResource() const { return m_texture; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    // Direct3D objects
    ID3D11ShaderResourceView* m_srv;
    ID3D11Resource* m_texture;
    int m_width;
    int m_height;
};
