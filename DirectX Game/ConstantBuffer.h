#pragma once
#include "Prerequisites.h"
#include <d3d11.h>

class DeviceContext;

class ConstantBuffer
{
public:
    // RAII constructor: allocate buffer of given size and optionally initialize with data
	ConstantBuffer(RenderSystem* system, UINT size_buffer = 0, const void* initial_data = nullptr);

	// Legacy load API (kept for compatibility) - will create buffer if not already created
	bool load(void* buffer, UINT size_buffer);
	void update(DeviceContext* context, void* buffer);
	bool release();
	~ConstantBuffer();

	// Returns true if the underlying D3D buffer was successfully created
	bool isValid() const;

private:
	ID3D11Buffer* m_buffer = nullptr;
	UINT m_size = 0;
    RenderSystem* m_system = nullptr;
private:
	friend class DeviceContext;
};

