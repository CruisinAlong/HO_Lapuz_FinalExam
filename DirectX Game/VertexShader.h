#pragma once
#include <d3d11.h>

class GraphicsEngine;
class DeviceContext;
class RenderSystem;

class VertexShader
{
public:
	VertexShader();
	void release();
	~VertexShader();
private:
    bool init(const void* shader_byte_code, size_t byte_code_size);
public:
	VertexShader(RenderSystem* system);
	VertexShader(RenderSystem* system, const void* shader_byte_code, size_t byte_code_size);
private:
	ID3D11VertexShader* m_vs;
private:
    friend class GraphicsEngine;
	friend class DeviceContext;
	friend class RenderSystem;
	RenderSystem* m_system = nullptr;
};

