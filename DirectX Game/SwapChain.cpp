#include "SwapChain.h"
#include "RenderSystem.h"
#include "Debug.h"
#include "ReferenceManager.h"

SwapChain::SwapChain() : m_swap_chain(nullptr), m_rtv(nullptr), m_depthStencilTex(nullptr), m_dsv(nullptr), m_system(nullptr)
{
}

SwapChain::SwapChain(RenderSystem* system) : m_swap_chain(nullptr), m_rtv(nullptr), m_depthStencilTex(nullptr), m_dsv(nullptr), m_system(system)
{
}

SwapChain::SwapChain(RenderSystem* system, HWND hwnd, UINT width, UINT height) : m_swap_chain(nullptr), m_rtv(nullptr), m_depthStencilTex(nullptr), m_dsv(nullptr), m_system(system)
{
    // Attempt to initialize; throw on failure to ensure RAII
    if (!init(hwnd, width, height)) {
        char buf[256];
        sprintf_s(buf, "SwapChain ctor: init failed for hwnd=%p (%u x %u)", hwnd, width, height);
        throw std::runtime_error(buf);
    }
}

SwapChain::~SwapChain()
{
    // Ensure resources are released
    release();
}

bool SwapChain::init(HWND hwnd, UINT width, UINT height)
{
    LOG("SwapChain::init hwnd=%p width=%u height=%u", hwnd, width, height);

    ID3D11Device* device = nullptr;
    IDXGIFactory* factory = nullptr;
    if (m_system) {
        device = m_system->getDevice();
        factory = m_system->getFactory();
    }
    if (!device || !factory) {
        LOG("SwapChain::init - missing device or factory");
        return false;
    }

    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.BufferCount = 1;
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 60;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = hwnd;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Windowed = TRUE;

    HRESULT hr = factory->CreateSwapChain(device, &desc, &m_swap_chain);

    if (FAILED(hr))
    {
        LOG("SwapChain::init CreateSwapChain failed HR=0x%08X", hr);
        return false;
    }

    // register swap chain object in reference manager
    ReferenceManager::acquire(m_swap_chain);

    ID3D11Texture2D* buffer = nullptr;
    hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer);

    if (FAILED(hr))
    {
        LOG("SwapChain::init GetBuffer failed HR=0x%08X", hr);
        return false;
    }

    hr = device->CreateRenderTargetView(buffer, NULL, &m_rtv);
    buffer->Release();

    if (FAILED(hr))
    {
        LOG("SwapChain::init CreateRenderTargetView failed HR=0x%08X", hr);
        return false;
    }

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&depthDesc, nullptr, &m_depthStencilTex);
    if (FAILED(hr))
    {
        LOG("SwapChain::init CreateTexture2D (depth) failed HR=0x%08X", hr);
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = device->CreateDepthStencilView(m_depthStencilTex, &dsvDesc, &m_dsv);
    if (FAILED(hr))
    {
        LOG("SwapChain::init CreateDepthStencilView failed HR=0x%08X", hr);
        if (m_depthStencilTex) { m_depthStencilTex->Release(); m_depthStencilTex = nullptr; }
        return false;
    }

    LOG("SwapChain::init succeeded m_rtv=%p m_dsv=%p", m_rtv, m_dsv);
    return true;
}

bool SwapChain::present(bool vsync)
{
    if (!m_swap_chain) {
        LOG("SwapChain::present called but m_swap_chain is null");
        return false;
    }
    m_swap_chain->Present(vsync ? 1 : 0, 0);
    return true;
}

bool SwapChain::release()
{
    if (m_dsv)
    {
        m_dsv->Release();
        m_dsv = nullptr;
    }
    if (m_depthStencilTex)
    {
        m_depthStencilTex->Release();
        m_depthStencilTex = nullptr;
    }
    if (m_rtv)
    {
        m_rtv->Release();
        m_rtv = nullptr;
    }
    if (m_swap_chain)
    {
        if (ReferenceManager::release(m_swap_chain)) {
            m_swap_chain->Release();
        }
        m_swap_chain = nullptr;
    }
    return true;
}
