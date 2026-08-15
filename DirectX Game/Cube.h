#pragma once
#include "Prerequisites.h"
#include "GameObject.h"

class VertexBuffer;
class IndexBuffer;
class VertexShader;
class PixelShader;
class ConstantBuffer;
class DeviceContext;
class Matrix4x4;

class Cube : public GameObject
{
public:
	Cube();
	virtual ~Cube();

    bool create();

	void setShaders(class VertexShader* vs, class PixelShader* ps);
    void setTexture(TexturePtr tex) { m_texture = tex; }
	void setBuffers(class VertexBuffer* vb, class IndexBuffer* ib);

	static bool InitSharedResources(class RenderSystem* rs);
	static void ReleaseSharedResources();
	static bool InitInstanceBuffer(class RenderSystem* rs, UINT maxInstances);
    static void ReleaseInstanceBuffer();
    static bool UpdateInstanceBuffer(ID3D11DeviceContext* d3dContext, const Matrix4x4* matrices, UINT count);
    static void RenderInstanced(ID3D11DeviceContext* d3dContext, UINT instanceCount);

	void update(float dt) override;

	void setView(const Matrix4x4& v);
	void setProjection(const Matrix4x4& p);

	void render() override;

	void destroy() override;

private:
	VertexBuffer* m_vb;
	IndexBuffer* m_ib;
	VertexShader* m_vs;
	PixelShader* m_ps;
	ConstantBuffer* m_cb;
    bool m_externalBuffers = false;
	TexturePtr m_texture;

	static VertexBuffer* s_vb;
	static IndexBuffer* s_ib;
	static VertexShader* s_vs;
	static PixelShader* s_ps;
	static bool s_initialized;
	static class InstanceBuffer* s_instanceBuffer;

	Matrix4x4 m_view;
	Matrix4x4 m_projection;

	float m_time;
};