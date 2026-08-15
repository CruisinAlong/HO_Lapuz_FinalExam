#include "CreditsScreen.h"
#include "CreditsScreen.h"
#include "ImGui/imgui.h"
#include "GraphicsEngine.h"
#include "RenderSystem.h"
#include <wincodec.h>
#include <wrl.h>
#include <memory>

using Microsoft::WRL::ComPtr;

CreditsScreen::CreditsScreen(AppWindow* app) : AUIScreen("Credits"), m_app(app)
{
    snprintf(m_buf, sizeof(m_buf), "Basil Lorenz C. Lapuz");
    m_srv = nullptr; m_width = 0; m_height = 0;
}

CreditsScreen::~CreditsScreen()
{
    releaseImage();
}

void CreditsScreen::releaseImage()
{
    if (m_srv) { m_srv->Release(); m_srv = nullptr; }
    m_width = m_height = 0;
}

bool CreditsScreen::loadImage()
{
    if (m_srv) return true; 
    const wchar_t* file = L"Images\\DLSU.png";

    GraphicsEngine* ge = GraphicsEngine::getInstance();
    if (!ge) return false;
    ID3D11Device* device = ge->getRenderSystem()->getDevice();
    if (!device) return false;

    HRESULT hr;
    ComPtr<IWICImagingFactory> wicFactory;
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr)) return false;

    ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(file, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return false;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return false;

    ComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return false;

    hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return false;

    UINT w, h;
    hr = converter->GetSize(&w, &h);
    if (FAILED(hr)) return false;

    std::unique_ptr<BYTE[]> pixels(new BYTE[w * h * 4]);
    hr = converter->CopyPixels(nullptr, w * 4, w * h * 4, pixels.get());
    if (FAILED(hr)) return false;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem = pixels.get();
    init.SysMemPitch = w * 4;

    ID3D11Texture2D* tex = nullptr;
    hr = device->CreateTexture2D(&desc, &init, &tex);
    if (FAILED(hr) || !tex) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device->CreateShaderResourceView(tex, &srvDesc, &m_srv);
    tex->Release();
    if (FAILED(hr)) { m_srv = nullptr; return false; }

    m_width = static_cast<int>(w);
    m_height = static_cast<int>(h);
    return true;
}

void CreditsScreen::drawUI(float deltaTime)
{
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

    if (!m_srv) {
        if (!loadImage()) {
            ImGui::Text("(logo load failed)");
        }
    }

    if (m_srv) {

        const float maxWidth = 200.0f;
        float imgW = (float)m_width;
        float imgH = (float)m_height;
        float scale = 1.0f;
        if (imgW > maxWidth) scale = maxWidth / imgW;
        ImVec2 imgSize(imgW * scale, imgH * scale);
        ImGui::Image((void*)m_srv, imgSize);
    }

    ImGui::Text("Developer: %s", m_buf);
    ImGui::Separator();
    ImGui::Text("Acknowledgements: This project was made for GDENG03.");
    ImGui::Text("FPS: %.1f", (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f);
    ImGui::End();
}
