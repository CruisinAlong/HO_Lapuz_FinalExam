#include "RenderSystem.h"
#include "SwapChain.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Debug.h"
#include <stdexcept>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

RenderSystem::RenderSystem()
{
    HRESULT res = 0;
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT num_driver_types = ARRAYSIZE(driverTypes);
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0
    };
    UINT num_feature_levels = ARRAYSIZE(featureLevels);

    for (UINT driver_type_index = 0; driver_type_index < num_driver_types; driver_type_index++)
    {
        res = D3D11CreateDevice(NULL, driverTypes[driver_type_index], NULL, NULL, featureLevels, num_feature_levels, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_imm_context);
        if (SUCCEEDED(res)) break;
    }

    if (FAILED(res))
    {
        char buf[128];
        sprintf_s(buf, "RenderSystem: D3D11CreateDevice failed HR=0x%08X", (unsigned int)res);
        throw std::runtime_error(buf);
    }

    m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&m_dxgiDevice);
    if (m_dxgiDevice) m_dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&m_dxgiAdapter);
    if (m_dxgiAdapter) m_dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&m_dxgiFactory);

    m_imm_device_context = DeviceContextPtr(new DeviceContext(m_imm_context), [](DeviceContext* p) { if (p) { p->release(); delete p; } });

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    res = m_d3dDevice->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(res))
    {
        if (m_imm_device_context) { m_imm_device_context.reset(); }
        if (m_dxgiFactory) { m_dxgiFactory->Release(); m_dxgiFactory = nullptr; }
        if (m_dxgiAdapter) { m_dxgiAdapter->Release(); m_dxgiAdapter = nullptr; }
        if (m_dxgiDevice) { m_dxgiDevice->Release(); m_dxgiDevice = nullptr; }
        if (m_imm_context) { m_imm_context->Release(); m_imm_context = nullptr; }
        if (m_d3dDevice) { m_d3dDevice->Release(); m_d3dDevice = nullptr; }
        char buf[128];
        sprintf_s(buf, "RenderSystem: CreateDepthStencilState failed HR=0x%08X", (unsigned int)res);
        throw std::runtime_error(buf);
    }

    if (m_imm_context && m_depthStencilState)
        m_imm_context->OMSetDepthStencilState(m_depthStencilState, 1);
    LOG("RenderSystem initialized");
}

RenderSystem::~RenderSystem()
{
    if (m_depthStencilState) { m_depthStencilState->Release(); m_depthStencilState = nullptr; }
    if (m_vsblob) { m_vsblob->Release(); m_vsblob = nullptr; }
    if (m_psblob) { m_psblob->Release(); m_psblob = nullptr; }
    if (m_vs) { m_vs->Release(); m_vs = nullptr; }
    if (m_ps) { m_ps->Release(); m_ps = nullptr; }

    if (m_imm_device_context)
    {
        m_imm_device_context.reset();
    }
    if (m_dxgiFactory) { m_dxgiFactory->Release(); m_dxgiFactory = nullptr; }
    if (m_dxgiAdapter) { m_dxgiAdapter->Release(); m_dxgiAdapter = nullptr; }
    if (m_dxgiDevice) { m_dxgiDevice->Release(); m_dxgiDevice = nullptr; }
    if (m_imm_context) { m_imm_context->Release(); m_imm_context = nullptr; }
    if (m_d3dDevice) { m_d3dDevice->Release(); m_d3dDevice = nullptr; }
}

SwapChain* RenderSystem::createSwapChain()
{
    return new SwapChain(this);
}

SwapChain* RenderSystem::createSwapChain(HWND hwnd, UINT width, UINT height)
{
    SwapChain* sc = new SwapChain(this);   
    if (!sc->init(hwnd, width, height)) {
        delete sc;
        char buf[256];
        sprintf_s(buf, "RenderSystem: SwapChain init failed for hwnd=%p (%u x %u)", hwnd, width, height); 
        throw std::runtime_error(buf); 
    }
    return sc;
}


SwapChainPtr RenderSystem::createSwapChainPtr()
{
    SwapChain* raw = createSwapChain();
    return SwapChainPtr(raw, [](SwapChain* p) { if (p) { p->release(); delete p; } });
}

SwapChainPtr RenderSystem::createSwapChainPtr(HWND hwnd, UINT width, UINT height)
{
    SwapChain* raw = nullptr;
    raw = createSwapChain(hwnd, width, height);
    return SwapChainPtr(raw, [](SwapChain* p) { if (p) { p->release(); delete p; } });
}

VertexBufferPtr RenderSystem::createVertexBufferPtr()
{
    VertexBuffer* raw = createVertexBuffer();
    return VertexBufferPtr(raw, [](VertexBuffer* p) { if (p) { p->release(); delete p; } });
}

VertexBufferPtr RenderSystem::createVertexBufferPtr(void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader)
{
    VertexBuffer* raw = createVertexBuffer(list_vertices, size_vertex, size_list, shader_byte_code, size_byte_shader);
    return VertexBufferPtr(raw, [](VertexBuffer* p) { if (p) { p->release(); delete p; } });
}

ConstantBufferPtr RenderSystem::createConstantBufferPtr(const void* initial_data, UINT size)
{
    ConstantBuffer* raw = createConstantBuffer(initial_data, size);
    return ConstantBufferPtr(raw, [](ConstantBuffer* p) { if (p) { p->release(); delete p; } });
}

ConstantBufferPtr RenderSystem::createConstantBufferPtr(RenderSystem* system, const void* initial_data, UINT size)
{
    ConstantBuffer* raw = createConstantBuffer(system, initial_data, size);
    return ConstantBufferPtr(raw, [](ConstantBuffer* p) { if (p) { p->release(); delete p; } });
}

IndexBufferPtr RenderSystem::createIndexBufferPtr()
{
    IndexBuffer* raw = createIndexBuffer();
    return IndexBufferPtr(raw, [](IndexBuffer* p) { if (p) { p->release(); delete p; } });
}

IndexBufferPtr RenderSystem::createIndexBufferPtr(void* list_indices, UINT size_index, UINT size_list)
{
    IndexBuffer* raw = createIndexBuffer(list_indices, size_index, size_list);
    return IndexBufferPtr(raw, [](IndexBuffer* p) { if (p) { p->release(); delete p; } });
}

VertexShaderPtr RenderSystem::createVertexShaderPtr(const void* shader_byte_code, size_t byte_code_size)
{
    VertexShader* raw = createVertexShader(shader_byte_code, byte_code_size);
    return VertexShaderPtr(raw, [](VertexShader* p) { if (p) { p->release(); delete p; } });
}

PixelShaderPtr RenderSystem::createPixelShaderPtr(const void* shader_byte_code, size_t byte_code_size)
{
    PixelShader* raw = createPixelShader(shader_byte_code, byte_code_size);
    return PixelShaderPtr(raw, [](PixelShader* p) { if (p) { p->release(); delete p; } });
}

DeviceContextPtr RenderSystem::getImmediateDeviceContext()
{
    return m_imm_device_context;
}

VertexBuffer* RenderSystem::createVertexBuffer()
{
    return new VertexBuffer(this);
}

VertexBuffer* RenderSystem::createVertexBuffer(void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader)
{
    VertexBuffer* vb = new VertexBuffer(this, list_vertices, size_vertex, size_list, shader_byte_code, size_byte_shader);
    if (vb->getSizeVertexList() == 0) { 
        delete vb; 
        char buf[256]; 
        sprintf_s(buf, "RenderSystem: VertexBuffer creation failed (size_vertex=%u size_list=%u)", size_vertex, size_list); 
        throw std::runtime_error(buf); 
    }
    return vb;
}

ConstantBuffer* RenderSystem::createConstantBuffer(const void* initial_data, UINT size)
{
    ConstantBuffer* cb = new ConstantBuffer(this, size, initial_data);
    if (!cb->isValid()) {
        delete cb;
        char buf[256];
        sprintf_s(buf, "RenderSystem: ConstantBuffer creation failed (size=%u)", size);
        throw std::runtime_error(buf);
    }
    return cb;
}

ConstantBuffer* RenderSystem::createConstantBuffer(RenderSystem* system, const void* initial_data, UINT size)
{
    return createConstantBuffer(initial_data, size);
}

IndexBuffer* RenderSystem::createIndexBuffer()
{
    return new IndexBuffer(this);
}

IndexBuffer* RenderSystem::createIndexBuffer(void* list_indices, UINT size_index, UINT size_list)
{
    IndexBuffer* ib = new IndexBuffer(this, list_indices, size_index, size_list);
    if (ib->getSizeIndexList() == 0) {
        delete ib;
        char buf[256];
        sprintf_s(buf, "RenderSystem: IndexBuffer creation failed (size_index=%u size_list=%u)", size_index, size_list);
        throw std::runtime_error(buf);
    }
    return ib;
}

VertexShader* RenderSystem::createVertexShader(const void* shader_byte_code, size_t byte_code_size)
{

    VertexShader* vs = new VertexShader(this, shader_byte_code, byte_code_size);
    return vs;
}

PixelShader* RenderSystem::createPixelShader(const void* shader_byte_code, size_t byte_code_size)
{
    PixelShader* ps = new PixelShader(this, shader_byte_code, byte_code_size);
    return ps;
}

bool RenderSystem::compileVertexShader(const wchar_t* file_name, const char* entry_point_name, void** shader_byte_code, size_t* byte_code_size)
{
    ID3DBlob* error_blob = nullptr;
    HRESULT hr = D3DCompileFromFile(file_name, nullptr, nullptr, entry_point_name, "vs_5_0", 0, 0, &m_blob, &error_blob);

    if (!SUCCEEDED(hr))
    {
        if (error_blob)
        {
            LOG("compileVertexShader: HLSL error:\n%s", static_cast<const char*>(error_blob->GetBufferPointer()));
            error_blob->Release();
        }
        else
        {
            LOG("compileVertexShader: D3DCompileFromFile failed for '%ls' entry '%s' HR=0x%08X", file_name, entry_point_name, hr);
        }
        return false;
    }

    if (error_blob) error_blob->Release();

    *shader_byte_code = m_blob->GetBufferPointer();
    *byte_code_size = m_blob->GetBufferSize();

    LOG("compileVertexShader: compiled '%ls' entry '%s' (%zu bytes)", file_name, entry_point_name, *byte_code_size);
    return true;
}

bool RenderSystem::compilePixelShader(const wchar_t* file_name, const char* entry_point_name, void** shader_byte_code, size_t* byte_code_size)
{
    ID3DBlob* error_blob = nullptr;
    HRESULT hr = D3DCompileFromFile(file_name, nullptr, nullptr, entry_point_name, "ps_5_0", 0, 0, &m_blob, &error_blob);

    if (!SUCCEEDED(hr))
    {
        if (error_blob)
        {
            LOG("compilePixelShader: HLSL error:\n%s", static_cast<const char*>(error_blob->GetBufferPointer()));
            error_blob->Release();
        }
        else
        {
            LOG("compilePixelShader: D3DCompileFromFile failed for '%ls' entry '%s' HR=0x%08X", file_name, entry_point_name, hr);
        }
        return false;
    }

    if (error_blob) error_blob->Release();

    *shader_byte_code = m_blob->GetBufferPointer();
    *byte_code_size = m_blob->GetBufferSize();

    LOG("compilePixelShader: compiled '%ls' entry '%s' (%zu bytes)", file_name, entry_point_name, *byte_code_size);
    return true;
}

void RenderSystem::releaseCompiledShader()
{
    if (m_blob)
    {
        m_blob->Release();
        m_blob = nullptr;
    }
}
