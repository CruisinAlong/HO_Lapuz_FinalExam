#pragma once
#include "GameObject.h"

class VertexBuffer;
class IndexBuffer;
class VertexShader;
class PixelShader;
class ConstantBuffer;
class Matrix4x4;

class Capsule : public GameObject
{
public:
    Capsule();
    virtual ~Capsule();

    bool create(int segments = 16, int rings = 16);
    void update(float dt) override;
    void setView(const Matrix4x4& v);
    void setProjection(const Matrix4x4& p);
    void render() override;
    void destroy() override;

    static bool InitSharedResources(class RenderSystem* rs, int segments = 16, int rings = 16);
    static void ReleaseSharedResources();
    static bool InitInstanceBuffer(class RenderSystem* rs, unsigned int maxInstances);
    static void ReleaseInstanceBuffer();
    static bool UpdateInstanceBuffer(struct ID3D11DeviceContext* d3dCtx, const Matrix4x4* matrices, unsigned int count);
    static void RenderInstanced(class DeviceContext* ctx, unsigned int instanceCount);

private:
    VertexBuffer* m_vb = nullptr;
    IndexBuffer* m_ib = nullptr;
    VertexShader* m_vs = nullptr;
    PixelShader* m_ps = nullptr;
    ConstantBuffer* m_cb = nullptr;
    Matrix4x4 m_view;
    Matrix4x4 m_projection;
    float m_time = 0.0f;
};
