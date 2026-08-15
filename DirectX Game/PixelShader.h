#pragma once
#include <d3d11.h>

class GraphicsEngine;
class DeviceContext;
class RenderSystem;

class PixelShader
{
public:
	PixelShader();
	void release();
	~PixelShader();
private:
	bool init(const void* shader_byte_code, size_t byte_code_size);
private:
	ID3D11PixelShader* m_ps;
private:
    friend class GraphicsEngine;
	friend class DeviceContext;
	friend class RenderSystem;
	RenderSystem* m_system = nullptr;
public:
	PixelShader(RenderSystem* system);
	PixelShader(RenderSystem* system, const void* shader_byte_code, size_t byte_code_size);
};

