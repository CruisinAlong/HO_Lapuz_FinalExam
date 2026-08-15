#pragma once
#include "GameObject.h"

class VertexBuffer;
class VertexShader;
class PixelShader;
class ConstantBuffer;

class Plane : public GameObject
{
public:
	Plane();
	virtual ~Plane();

	bool create();
	void update(float dt) override;
	void render() override;
	void destroy() override;

	void setView(const class Matrix4x4& v);
	void setProjection(const class Matrix4x4& p);

private:
	VertexBuffer* m_vb;
	VertexShader* m_vs;
	PixelShader* m_ps;
	ConstantBuffer* m_cb;

	Matrix4x4 m_view;
	Matrix4x4 m_projection;
	float m_time;
public:
    static bool InitSharedResources(class RenderSystem* rs);
	static void ReleaseSharedResources();
	static bool InitInstanceBuffer(class RenderSystem* rs, unsigned int maxInstances);
	static void ReleaseInstanceBuffer();
	static bool UpdateInstanceBuffer(struct ID3D11DeviceContext* d3dCtx, const Matrix4x4* matrices, unsigned int count);
	static void RenderInstanced(class DeviceContext* ctx, unsigned int instanceCount);
};

