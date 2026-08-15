#include "Texture.h"
#include "GraphicsEngine.h"
#include "RenderSystem.h"
#include <stdexcept>

// DirectXTex
#include <DirectXTex.h>
using namespace DirectX;

Texture::Texture(const std::wstring& path) : Resource(path), m_srv(nullptr), m_texture(nullptr), m_width(0), m_height(0)
{
    ScratchImage image;
    HRESULT hr = LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, nullptr, image);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to load image from file");
    }

    GraphicsEngine* ge = GraphicsEngine::getInstance();
    if (!ge) throw std::runtime_error("GraphicsEngine not initialized");
    RenderSystem* rs = ge->getRenderSystem();
    if (!rs) throw std::runtime_error("RenderSystem not available");

    ID3D11Device* device = rs->getDevice();
    if (!device) throw std::runtime_error("D3D11 device not available");

    const Image* img = image.GetImage(0,0,0);
    if (!img) throw std::runtime_error("Invalid image data");

    // Create texture resource from image
    hr = CreateTexture(device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &m_texture);
    if (FAILED(hr) || !m_texture) {
        throw std::runtime_error("Failed to create D3D11 texture from image");
    }

    // Create SRV
    ID3D11Resource* res = m_texture;
    hr = device->CreateShaderResourceView(res, nullptr, &m_srv);
    if (FAILED(hr) || !m_srv) {
        if (m_texture) { m_texture->Release(); m_texture = nullptr; }
        throw std::runtime_error("Failed to create SRV for texture");
    }

    // Fill width/height from metadata if available
    TexMetadata md = image.GetMetadata();
    m_width = static_cast<int>(md.width);
    m_height = static_cast<int>(md.height);
}

Texture::~Texture()
{
    if (m_srv) { m_srv->Release(); m_srv = nullptr; }
    if (m_texture) { m_texture->Release(); m_texture = nullptr; }
}
