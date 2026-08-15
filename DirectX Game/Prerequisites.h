#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <memory>

class RenderSystem;
class DeviceContext;
class SwapChain;
class VertexBuffer;
class ConstantBuffer;
class IndexBuffer;
class VertexShader;
class PixelShader;
class Resource;
class ResourceManager;
class Texture;
class Mesh;

using UINT = unsigned int;


using SwapChainPtr = std::shared_ptr<SwapChain>;
using VertexBufferPtr = std::shared_ptr<VertexBuffer>;
using ConstantBufferPtr = std::shared_ptr<ConstantBuffer>;
using IndexBufferPtr = std::shared_ptr<IndexBuffer>;
using VertexShaderPtr = std::shared_ptr<VertexShader>;
using PixelShaderPtr = std::shared_ptr<PixelShader>;
using DeviceContextPtr = std::shared_ptr<DeviceContext>;
using ResourcePtr = std::shared_ptr<Resource>;
using TexturePtr = std::shared_ptr<Texture>;
using MeshPtr = std::shared_ptr<Mesh>;
