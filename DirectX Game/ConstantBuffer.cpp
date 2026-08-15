#include "ConstantBuffer.h"
#include "ConstantBuffer.h"
#include "RenderSystem.h"
#include "DeviceContext.h"
#include "Debug.h"
#include "GraphicsEngine.h"
#include "ReferenceManager.h"

ConstantBuffer::ConstantBuffer(RenderSystem* system, UINT size_buffer, const void* initial_data) : m_buffer(nullptr), m_size(0), m_system(system)
{
    LOG("ConstantBuffer::ctor size=%u", size_buffer);
    if (size_buffer == 0) return;

    UINT alignedSize = ((size_buffer + 15) / 16) * 16;

    D3D11_BUFFER_DESC buff_desc = {};
    buff_desc.Usage = D3D11_USAGE_DEFAULT;
    buff_desc.ByteWidth = alignedSize;
    buff_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buff_desc.CPUAccessFlags = 0;
    buff_desc.MiscFlags = 0;
    buff_desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA init_data = {};
    init_data.pSysMem = initial_data;

    ID3D11Device* device = nullptr;
    if (m_system) device = m_system->getDevice();
    if (!device) {
        LOG("ConstantBuffer::ctor - no D3D device available");
        return;
    }

    HRESULT hr = device->CreateBuffer(&buff_desc, initial_data ? &init_data : nullptr, &m_buffer);
    if (FAILED(hr)) {
        LOG("ConstantBuffer::ctor CreateBuffer failed HR=0x%08X", hr);
        m_buffer = nullptr;
        m_size = 0;
        return;
    }

    ReferenceManager::acquire(m_buffer);

    m_size = alignedSize;
    LOG("ConstantBuffer::ctor created m_buffer=%p size=%u", m_buffer, m_size);
}

ConstantBuffer::~ConstantBuffer()
{
    if (m_buffer)
    {
        if (ReferenceManager::release(m_buffer)) {
            m_buffer->Release();
        }
        m_buffer = nullptr;
    }
    LOG("ConstantBuffer::dtor");
}

bool ConstantBuffer::isValid() const
{
    return m_buffer != nullptr;
}

bool ConstantBuffer::load(void* buffer, UINT size_buffer)
{
    LOG("ConstantBuffer::load size_buffer=%u", size_buffer);

    if (m_buffer)
    {
        m_buffer->Release();
        m_buffer = nullptr;
        m_size = 0;
    }

    if (size_buffer == 0) return false;

    UINT alignedSize = ((size_buffer + 15) / 16) * 16;
    LOG("ConstantBuffer::load alignedSize=%u", alignedSize);

    D3D11_BUFFER_DESC buff_desc = {};
    buff_desc.Usage = D3D11_USAGE_DEFAULT;
    buff_desc.ByteWidth = alignedSize;
    buff_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buff_desc.CPUAccessFlags = 0;
    buff_desc.MiscFlags = 0;
    buff_desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA init_data = {};
    init_data.pSysMem = buffer;

    ID3D11Device* device = nullptr;
    if (m_system) device = m_system->getDevice();
    if (!device) {
        GraphicsEngine* ge = GraphicsEngine::get();
        if (ge && ge->getRenderSystem()) device = ge->getRenderSystem()->getDevice();
    }
    if (!device) {
        LOG("ConstantBuffer::load - no D3D device available");
        return false;
    }

    HRESULT hr = device->CreateBuffer(&buff_desc, &init_data, &m_buffer);
    if (FAILED(hr)) {
        LOG("ConstantBuffer::load CreateBuffer failed HR=0x%08X", hr);
        m_buffer = nullptr;
        return false;
    }

    m_size = alignedSize;
    ReferenceManager::acquire(m_buffer);
    LOG("ConstantBuffer::load succeeded m_buffer=%p", m_buffer);
    return true;
}

void ConstantBuffer::update(DeviceContext* context, void* buffer)
{
    if (!m_buffer || !context) {
        LOG("ConstantBuffer::update called with null m_buffer or context");
        return;
    }
    context->m_d3dContext->UpdateSubresource(this->m_buffer, 0, NULL, buffer, 0, 0);
}

bool ConstantBuffer::release()
{
    if (m_buffer)
    {
        m_buffer->Release();
        m_buffer = nullptr;
        m_size = 0;
    }
    LOG("ConstantBuffer::release");
    return true;
}
