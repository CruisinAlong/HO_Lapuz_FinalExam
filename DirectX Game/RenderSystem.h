#pragma once
#include <d3d11.h>
#include <stdexcept>
#include "DeviceContext.h"
#include "SwapChain.h"
class VertexBuffer;
class ConstantBuffer;
class IndexBuffer;
class VertexShader;
class PixelShader;

class RenderSystem
{
public:
    RenderSystem();
    ~RenderSystem();


    SwapChain* createSwapChain();
    SwapChain* createSwapChain(HWND hwnd, UINT width, UINT height);
    // Shared_ptr-based helpers (new API - forwards to existing implementations)
    SwapChainPtr createSwapChainPtr();
    SwapChainPtr createSwapChainPtr(HWND hwnd, UINT width, UINT height);

    // Resource creation helpers returning shared_ptr-managed objects
    VertexBufferPtr createVertexBufferPtr();
    VertexBufferPtr createVertexBufferPtr(void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader);

    ConstantBufferPtr createConstantBufferPtr(const void* initial_data = nullptr, UINT size = 0);
    ConstantBufferPtr createConstantBufferPtr(RenderSystem* system, const void* initial_data, UINT size);

    IndexBufferPtr createIndexBufferPtr();
    IndexBufferPtr createIndexBufferPtr(void* list_indices, UINT size_index, UINT size_list);

    VertexShaderPtr createVertexShaderPtr(const void* shader_byte_code, size_t byte_code_size);
    PixelShaderPtr createPixelShaderPtr(const void* shader_byte_code, size_t byte_code_size);
    DeviceContextPtr getImmediateDeviceContext();
    VertexBuffer* createVertexBuffer();
    ConstantBuffer* createConstantBuffer(const void* initial_data = nullptr, UINT size = 0);
    ConstantBuffer* createConstantBuffer(RenderSystem* system, const void* initial_data, UINT size);
    VertexBuffer* createVertexBuffer(void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader);
    IndexBuffer* createIndexBuffer();
    IndexBuffer* createIndexBuffer(void* list_indices, UINT size_index, UINT size_list);
    VertexShader* createVertexShader(const void* shader_byte_code, size_t byte_code_size);
    PixelShader* createPixelShader(const void* shader_byte_code, size_t byte_code_size);

    bool compileVertexShader(const wchar_t* file_name, const char* entry_point_name, void** shader_byte_code, size_t* byte_code_size);
    bool compilePixelShader(const wchar_t* file_name, const char* entry_point_name, void** shader_byte_code, size_t* byte_code_size);
    void releaseCompiledShader();

    // Accessors for lower-level code that needs device/factory
    ID3D11Device* getDevice() { return m_d3dDevice; }
    IDXGIFactory* getFactory() { return m_dxgiFactory; }
    ID3D11DeviceContext* getContext() { return m_imm_context; }

private:
    // Internal D3D objects
    DeviceContextPtr m_imm_device_context = nullptr;
    ID3D11Device* m_d3dDevice = nullptr;
    ID3D11DeviceContext* m_imm_context = nullptr;
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;
    ID3D11DepthStencilState* m_depthStencilState = nullptr;

    IDXGIDevice* m_dxgiDevice = nullptr;
    IDXGIAdapter* m_dxgiAdapter = nullptr;
    IDXGIFactory* m_dxgiFactory = nullptr;

    ID3D11VertexShader* m_vs = nullptr;
    ID3D11PixelShader* m_ps = nullptr;

    ID3DBlob* m_vsblob = nullptr;
    ID3DBlob* m_psblob = nullptr;
    ID3DBlob* m_blob = nullptr;

    // Allow resource classes access to internals
    friend class SwapChain;
    friend class VertexBuffer;
    friend class ConstantBuffer;
    friend class IndexBuffer;
    friend class VertexShader;
    friend class PixelShader;
    // Allow Texture access if needed for low-level creation
    friend class Texture;
};
