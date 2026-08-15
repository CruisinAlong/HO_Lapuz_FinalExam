#pragma once
#include <d3d11.h>
#include "Prerequisites.h"

class RenderSystem;
class DeviceContext;

/// Simple dynamic instance buffer that stores tightly-packed instance data (e.g. world matrices).
class InstanceBuffer
{
public:
    InstanceBuffer();
    InstanceBuffer(RenderSystem* rs);
    ~InstanceBuffer();

    // Create buffer able to hold up to maxInstances entries of size stride bytes each.
    bool create(RenderSystem* rs, UINT maxInstances, UINT stride);

    // Update first 'count' entries with data (data must be at least count * stride bytes).
    // Uses Map/Unmap for dynamic write (discard).
    bool update(ID3D11DeviceContext* d3dContext, const void* data, UINT count);

    // Raw D3D buffer getter (used by DeviceContext to bind)
    ID3D11Buffer* getBuffer() const { return (m_lastWritten >= 0) ? m_buffers[m_lastWritten] : nullptr; }
    UINT getStride() const { return m_stride; }
    UINT getMaxInstances() const { return m_maxInstances; }

    bool release();

private:
    // Triple-buffer the instance buffer to avoid GPU sync when updating each frame.
    static const UINT RING_COUNT = 3;
    ID3D11Buffer* m_buffers[RING_COUNT] = { nullptr, nullptr, nullptr };
    UINT m_currentBuffer = 0; // index of next buffer to map/write
    int m_lastWritten = -1;   // index of last written buffer (returns from getBuffer)
    UINT m_stride = 0;
    UINT m_maxInstances = 0;
    RenderSystem* m_system = nullptr;
};

