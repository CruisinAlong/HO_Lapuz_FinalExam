#pragma once
#include "Prerequisites.h"

class DeviceContext;

class VertexBuffer
{
public:
	VertexBuffer();
	VertexBuffer(RenderSystem* system);
	VertexBuffer(RenderSystem* system, void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader);

	bool load(void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader);
	UINT getSizeVertexList();
	bool release();
	~VertexBuffer();
private:
	RenderSystem* m_system = nullptr;
	UINT m_size_vertex = 0;
	UINT m_size_list = 0;

	ID3D11Buffer* m_buffer = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
private:
	friend class DeviceContext;
};

