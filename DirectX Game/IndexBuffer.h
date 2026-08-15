
#pragma once
#include "Prerequisites.h"

class DeviceContext;

class IndexBuffer
{
public:
	IndexBuffer();
    // RAII constructor: create index buffer from data, system may be nullptr
	IndexBuffer(RenderSystem* system, void* list_indices, UINT size_index, UINT size_list);
	// Construct with system only (deferred init)
	IndexBuffer(RenderSystem* system);
	bool load(void* list_indices, UINT size_index, UINT size_list);
	UINT getSizeIndexList();
	bool release();
	~IndexBuffer();

private:
	RenderSystem* m_system = nullptr;
	UINT m_size_index = 0;
	UINT m_size_list = 0;

	ID3D11Buffer* m_buffer = nullptr;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;

private:
	friend class DeviceContext;
};
