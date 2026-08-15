#pragma once
#include <d3d11.h>
#include "Prerequisites.h"

class RenderSystem;
class DeviceContext;

class InstanceBuffer
{
public:
    InstanceBuffer();
    InstanceBuffer(RenderSystem* rs);
    ~InstanceBuffer();

    bool create(RenderSystem* rs, UINT maxInstances, UINT stride);

    bool update(ID3D11DeviceContext* d3dContext, const void* data, UINT count);

    ID3D11Buffer* getBuffer() const { return (m_lastWritten >= 0) ? m_buffers[m_lastWritten] : nullptr; }
    UINT getStride() const { return m_stride; }
    UINT getMaxInstances() const { return m_maxInstances; }

    bool release();

private:
    static const UINT RING_COUNT = 3;
    ID3D11Buffer* m_buffers[RING_COUNT] = { nullptr, nullptr, nullptr };
    UINT m_currentBuffer = 0; 
    int m_lastWritten = -1;   
    UINT m_stride = 0;
    UINT m_maxInstances = 0;
    RenderSystem* m_system = nullptr;
};

