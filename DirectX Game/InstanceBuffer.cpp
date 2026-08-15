#include "InstanceBuffer.h"
#include "RenderSystem.h"
#include "Debug.h"
#include "Matrix4x4.h"
#include <cstring>

InstanceBuffer::InstanceBuffer() : m_stride(0), m_maxInstances(0), m_system(nullptr), m_currentBuffer(0), m_lastWritten(-1) {}
InstanceBuffer::InstanceBuffer(RenderSystem* rs) : m_stride(0), m_maxInstances(0), m_system(rs), m_currentBuffer(0), m_lastWritten(-1) {}
InstanceBuffer::~InstanceBuffer() { release(); }

bool InstanceBuffer::create(RenderSystem* rs, UINT maxInstances, UINT stride)
{
    if (!rs) {
        return false;
    }
    m_system = rs;
    release();

    m_stride = stride;
    m_maxInstances = maxInstances;

    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = (UINT)(stride * maxInstances);
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; 
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = stride;

    ID3D11Device* device = rs->getDevice();
    if (!device) return false;

    for (UINT i = 0; i < RING_COUNT; ++i) {
        HRESULT hr = device->CreateBuffer(&desc, nullptr, &m_buffers[i]);
        if (FAILED(hr)) {

            for (UINT j = 0; j < i; ++j) {
                if (m_buffers[j]) { m_buffers[j]->Release(); m_buffers[j] = nullptr; }
            }
            return false;
        }

    }

    m_currentBuffer = 0;
    m_lastWritten = -1;
    return true;
}

bool InstanceBuffer::update(ID3D11DeviceContext* ctx, const void* data, UINT count)
{
    if (!ctx || !data) return false;
    if (count == 0) return true;
    if (count > m_maxInstances) return false;
    UINT idx = m_currentBuffer;
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT hr = ctx->Map(m_buffers[idx], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);


    size_t byteCount = static_cast<size_t>(count) * m_stride;
    std::memcpy(mapped.pData, data, byteCount);

    bool identical = (std::memcmp(mapped.pData, data, byteCount) == 0);

    Matrix4x4 srcFirst = reinterpret_cast<const Matrix4x4*>(data)[0];
    Matrix4x4 mappedFirst = reinterpret_cast<Matrix4x4*>(mapped.pData)[0];
    Vector3D srcTr = srcFirst.getTranslation();
    Vector3D mappedTr = mappedFirst.getTranslation();

    ctx->Unmap(m_buffers[idx], 0);

    m_lastWritten = static_cast<int>(idx);
    m_currentBuffer = (idx + 1) % RING_COUNT;

    return true;
}

bool InstanceBuffer::release()
{
    for (UINT i = 0; i < RING_COUNT; ++i) {
        if (m_buffers[i]) {
            m_buffers[i]->Release();
            m_buffers[i] = nullptr;
        }
    }
    m_stride = 0;
    m_maxInstances = 0;
    m_system = nullptr;
    return true;
}
