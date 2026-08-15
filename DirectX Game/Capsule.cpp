#include "Capsule.h"
#include "GraphicsEngine.h"
#include "Capsule.h"
#include "RenderSystem.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ShaderLibrary.h"
#include "Vector2D.h"
#include "ConstantBuffer.h"
#include "DeviceContext.h"
#include "Matrix4x4.h"
#include "Vector3D.h"
#include <vector>
#include <cmath>
#include "InstanceBuffer.h"
#include "RenderSystem.h"
#include "ShaderLibrary.h"
#include "ShaderNames.h"
#include "Debug.h"
#include <d3d11.h>

struct VertexC
{
    Vector3D position;
    Vector2D texcoord;
};

__declspec(align(16))
struct CBC
{
    Matrix4x4 world;
    Matrix4x4 view;
    Matrix4x4 projection;
    float time;
    float padding[3];
};

Capsule::Capsule() : m_vb(nullptr), m_ib(nullptr), m_vs(nullptr), m_ps(nullptr), m_cb(nullptr), m_time(0.0f)
{
    m_view.SetIdentity();
    m_projection.SetIdentity();
}

// ----------------------
// Shared resources & instancing for Capsule
// ----------------------
static VertexBuffer* s_capsule_vb = nullptr;
static IndexBuffer*  s_capsule_ib = nullptr;
static VertexShader* s_capsule_vs = nullptr;
static PixelShader*  s_capsule_ps = nullptr;
static ID3D11InputLayout* s_capsule_instancedLayout = nullptr;
static bool s_capsule_initialized = false;
static InstanceBuffer* s_capsule_instanceBuffer = nullptr;
static VertexShader* s_capsule_vs_instanced = nullptr;

bool Capsule::InitSharedResources(RenderSystem* rs, int segments, int rings)
{
    if (s_capsule_initialized) return true;
    if (!rs) return false;

    Capsule proto;
    if (!proto.create(segments, rings)) return false;

    s_capsule_vb = proto.m_vb; proto.m_vb = nullptr;
    s_capsule_ib = proto.m_ib; proto.m_ib = nullptr;
    s_capsule_vs = proto.m_vs; proto.m_vs = nullptr;
    s_capsule_ps = proto.m_ps; proto.m_ps = nullptr;
    if (proto.m_cb) { proto.m_cb->release(); delete proto.m_cb; proto.m_cb = nullptr; }

    // Compile instanced vertex shader and create instanced input layout
    void* vs_blob_inst = nullptr; size_t vs_size_inst = 0;
    if (rs->compileVertexShader(L"VertexShader.hlsl", "vsmain_instanced", &vs_blob_inst, &vs_size_inst)) {
        D3D11_INPUT_ELEMENT_DESC layoutInst[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            // instance matrix rows -> use TEXCOORD1..4
            { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
        };
        ID3D11Device* device = rs->getDevice();
        if (device) {
            HRESULT hr = device->CreateInputLayout(layoutInst, ARRAYSIZE(layoutInst), vs_blob_inst, vs_size_inst, &s_capsule_instancedLayout);
            if (FAILED(hr)) {
                LOG("Capsule::InitSharedResources CreateInputLayout failed HR=0x%08X", hr);
                s_capsule_instancedLayout = nullptr;
            }
        }
        s_capsule_vs_instanced = rs->createVertexShader(vs_blob_inst, vs_size_inst);
        rs->releaseCompiledShader();
    }

    s_capsule_initialized = true;
    return true;
}

void Capsule::ReleaseSharedResources()
{
    s_capsule_initialized = false;
    if (s_capsule_vb) { s_capsule_vb->release(); delete s_capsule_vb; s_capsule_vb = nullptr; }
    if (s_capsule_ib) { s_capsule_ib->release(); delete s_capsule_ib; s_capsule_ib = nullptr; }
    if (s_capsule_vs) { s_capsule_vs->release(); s_capsule_vs = nullptr; }
    if (s_capsule_ps) { s_capsule_ps->release(); s_capsule_ps = nullptr; }
    if (s_capsule_instancedLayout) { s_capsule_instancedLayout->Release(); s_capsule_instancedLayout = nullptr; }
    if (s_capsule_vs_instanced) { s_capsule_vs_instanced->release(); s_capsule_vs_instanced = nullptr; }
}

bool Capsule::InitInstanceBuffer(RenderSystem* rs, unsigned int maxInstances)
{
    if (!rs) return false;
    if (s_capsule_instanceBuffer) return true;
    s_capsule_instanceBuffer = new InstanceBuffer();
    if (!s_capsule_instanceBuffer) return false;
    if (!s_capsule_instanceBuffer->create(rs, maxInstances, sizeof(Matrix4x4))) {
        delete s_capsule_instanceBuffer;
        s_capsule_instanceBuffer = nullptr;
        return false;
    }
    return true;
}

void Capsule::ReleaseInstanceBuffer()
{
    if (s_capsule_instanceBuffer) {
        s_capsule_instanceBuffer->release();
        delete s_capsule_instanceBuffer;
        s_capsule_instanceBuffer = nullptr;
    }
}

bool Capsule::UpdateInstanceBuffer(ID3D11DeviceContext* d3dCtx, const Matrix4x4* matrices, unsigned int count)
{
    if (!s_capsule_instanceBuffer) return false;
    return s_capsule_instanceBuffer->update(d3dCtx, matrices, count);
}

void Capsule::RenderInstanced(DeviceContext* ctx, unsigned int instanceCount)
{
    if (!ctx || instanceCount == 0 || !s_capsule_instanceBuffer) return;
    if (!s_capsule_vb || !s_capsule_ib) return;

    if (s_capsule_vs_instanced) ctx->setVertexShader(s_capsule_vs_instanced);
    else if (s_capsule_vs) ctx->setVertexShader(s_capsule_vs);
    if (s_capsule_ps) ctx->setPixelShader(s_capsule_ps);
    ctx->setVertexBuffer(s_capsule_vb);
    if (s_capsule_instancedLayout) ctx->setInputLayout(s_capsule_instancedLayout);
    ctx->setIndexBuffer(s_capsule_ib);

    ctx->setInstanceBuffer(s_capsule_instanceBuffer->getBuffer(), s_capsule_instanceBuffer->getStride());
    ctx->drawIndexedInstanced(s_capsule_ib->getSizeIndexList(), instanceCount, 0, 0);
    ctx->setInstanceBuffer(nullptr, 0);
}



Capsule::~Capsule()
{
    destroy();
}

bool Capsule::create(int segments, int rings)
{
    GraphicsEngine* graphics = GraphicsEngine::getInstance();
    if (!graphics) return false;

    const float radius = 0.5f; // hemisphere radius
    const float cylinderHalf = 0.5f; // half length of center cylinder (cylinder length = 1.0)
    const float PI = 3.14159265358979323846f;

    std::vector<VertexC> vertices;
    std::vector<unsigned int> indices;

    // Build a proper capsule: top pole, top hemisphere rings, cylinder (top & bottom rings), bottom hemisphere rings, bottom pole
    // Top pole
    Vector3D topColor(0.4f, 0.8f, 0.3f);
    vertices.push_back({ Vector3D(0.0f, cylinderHalf + radius, 0.0f), Vector2D(0.0f, 0.0f) });
    unsigned int topPoleIndex = 0;

    // Top hemisphere rings (excluding equator)
    unsigned int topFirstRing = static_cast<unsigned int>(vertices.size());
    for (int r = 1; r <= rings; ++r) {
        float phi = (static_cast<float>(r) * (PI * 0.5f)) / static_cast<float>(rings); // 0..pi/2
        float y = cylinderHalf + radius * std::cosf(phi);
        float ringRadius = radius * std::sinf(phi);
        for (int s = 0; s < segments; ++s) {
            float theta = (2.0f * PI * s) / static_cast<float>(segments);
            float x = ringRadius * std::cosf(theta);
            float z = ringRadius * std::sinf(theta);
            vertices.push_back({ Vector3D(x, y, z), Vector2D(0.0f, 0.0f) });
        }
    }

    // Cylinder top ring (y = +cylinderHalf)
    unsigned int cylTopStart = static_cast<unsigned int>(vertices.size());
    for (int s = 0; s < segments; ++s) {
        float theta = (2.0f * PI * s) / static_cast<float>(segments);
        float x = radius * std::cosf(theta);
        float z = radius * std::sinf(theta);
        vertices.push_back({ Vector3D(x, cylinderHalf, z), Vector2D(0.0f, 0.0f) });
    }

    // Cylinder bottom ring (y = -cylinderHalf)
    unsigned int cylBottomStart = static_cast<unsigned int>(vertices.size());
    for (int s = 0; s < segments; ++s) {
        float theta = (2.0f * PI * s) / static_cast<float>(segments);
        float x = radius * std::cosf(theta);
        float z = radius * std::sinf(theta);
        vertices.push_back({ Vector3D(x, -cylinderHalf, z), Vector2D(0.0f, 0.0f) });
    }

    // Bottom hemisphere rings (excluding equator)
    unsigned int bottomFirstRing = static_cast<unsigned int>(vertices.size());
    for (int r = 1; r <= rings; ++r) {
        float phi = (static_cast<float>(r) * (PI * 0.5f)) / static_cast<float>(rings); // 0..pi/2
        float y = -cylinderHalf - radius * std::cosf(phi);
        float ringRadius = radius * std::sinf(phi);
        for (int s = 0; s < segments; ++s) {
            float theta = (2.0f * PI * s) / static_cast<float>(segments);
            float x = ringRadius * std::cosf(theta);
            float z = ringRadius * std::sinf(theta);
            vertices.push_back({ Vector3D(x, y, z), Vector2D(0.0f, 0.0f) });
        }
    }

    // Bottom pole
    unsigned int bottomPoleIndex = static_cast<unsigned int>(vertices.size());
    vertices.push_back({ Vector3D(0.0f, -cylinderHalf - radius, 0.0f), Vector2D(0.0f, 0.0f) });

    // Build indices
    // Top cap (triangles from top pole to first ring)
    if (rings > 0) {
        for (int s = 0; s < segments; ++s) {
            unsigned int a = topFirstRing + s;
            unsigned int b = topFirstRing + ((s + 1) % segments);
            indices.push_back(topPoleIndex);
            indices.push_back(b);
            indices.push_back(a);
        }
    } else {
        // no hemisphere rings - connect pole to cylinder top directly
        for (int s = 0; s < segments; ++s) {
            unsigned int a = cylTopStart + s;
            unsigned int b = cylTopStart + ((s + 1) % segments);
            indices.push_back(topPoleIndex);
            indices.push_back(b);
            indices.push_back(a);
        }
    }

    // Top hemisphere rings
    unsigned int prevRing = topFirstRing;
    for (int r = 0; r < rings - 1; ++r) {
        unsigned int nextRing = topFirstRing + (r + 1) * segments;
        for (int s = 0; s < segments; ++s) {
            unsigned int a = prevRing + s;
            unsigned int b = prevRing + ((s + 1) % segments);
            unsigned int c = nextRing + s;
            unsigned int d = nextRing + ((s + 1) % segments);
            indices.push_back(a); indices.push_back(c); indices.push_back(b);
            indices.push_back(b); indices.push_back(c); indices.push_back(d);
        }
        prevRing = nextRing;
    }

    // Connect last top hemisphere ring to cylinder top
    if (rings > 0) {
        unsigned int lastTopRing = topFirstRing + (rings - 1) * segments;
        for (int s = 0; s < segments; ++s) {
            unsigned int a = lastTopRing + s;
            unsigned int b = lastTopRing + ((s + 1) % segments);
            unsigned int c = cylTopStart + s;
            unsigned int d = cylTopStart + ((s + 1) % segments);
            indices.push_back(a); indices.push_back(c); indices.push_back(b);
            indices.push_back(b); indices.push_back(c); indices.push_back(d);
        }
    }

    // Cylinder side between cylTopStart and cylBottomStart
    for (int s = 0; s < segments; ++s) {
        unsigned int a = cylTopStart + s;
        unsigned int b = cylTopStart + ((s + 1) % segments);
        unsigned int c = cylBottomStart + s;
        unsigned int d = cylBottomStart + ((s + 1) % segments);
        indices.push_back(a); indices.push_back(c); indices.push_back(b);
        indices.push_back(b); indices.push_back(c); indices.push_back(d);
    }

    // Connect cylinder bottom to bottom hemisphere first ring
    if (rings > 0) {
        unsigned int firstBottomRing = bottomFirstRing;
        for (int s = 0; s < segments; ++s) {
            unsigned int a = cylBottomStart + s;
            unsigned int b = cylBottomStart + ((s + 1) % segments);
            unsigned int c = firstBottomRing + s;
            unsigned int d = firstBottomRing + ((s + 1) % segments);
            indices.push_back(a); indices.push_back(c); indices.push_back(b);
            indices.push_back(b); indices.push_back(c); indices.push_back(d);
        }

        // Bottom hemisphere rings
        prevRing = firstBottomRing;
        for (int r = 0; r < rings - 1; ++r) {
            unsigned int nextRing = firstBottomRing + (r + 1) * segments;
            for (int s = 0; s < segments; ++s) {
                unsigned int a = prevRing + s;
                unsigned int b = prevRing + ((s + 1) % segments);
                unsigned int c = nextRing + s;
                unsigned int d = nextRing + ((s + 1) % segments);
                indices.push_back(a); indices.push_back(c); indices.push_back(b);
                indices.push_back(b); indices.push_back(c); indices.push_back(d);
            }
            prevRing = nextRing;
        }

        // Last bottom ring to bottom pole
        unsigned int lastBottomRing = firstBottomRing + (rings - 1) * segments;
        for (int s = 0; s < segments; ++s) {
            unsigned int a = lastBottomRing + s;
            unsigned int b = lastBottomRing + ((s + 1) % segments);
            indices.push_back(a); indices.push_back(b); indices.push_back(bottomPoleIndex);
        }
    } else {
        // no hemisphere rings - connect cylinder bottom to bottom pole
        for (int s = 0; s < segments; ++s) {
            unsigned int a = cylBottomStart + s;
            unsigned int b = cylBottomStart + ((s + 1) % segments);
            indices.push_back(a); indices.push_back(b); indices.push_back(bottomPoleIndex);
        }
    }

    RenderSystem* rs = graphics->getRenderSystem();
    if (!rs) return false;

    m_ib = rs->createIndexBuffer(indices.data(), sizeof(unsigned int), static_cast<UINT>(indices.size()));
    if (!m_ib) return false;

    // Use ShaderLibrary to obtain the correct vertex bytecode (for input layout) and shaders
    void* vs_blob = nullptr; size_t vs_size = 0;
    ShaderLibrary* sl = ShaderLibrary::getInstance();
    if (!sl) return false;
    sl->requestVertexShaderData(L"BasicVertexShader.hlsl", &vs_blob, &vs_size);
    if (!vs_blob || vs_size == 0) return false;

    // create vertex buffer using vertex shader bytecode so the input layout matches
    m_vb = rs->createVertexBuffer(const_cast<void*>(reinterpret_cast<const void*>(vertices.data())), sizeof(VertexC), static_cast<UINT>(vertices.size()), vs_blob, vs_size);
    // free the copied bytecode returned by ShaderLibrary
    delete[] reinterpret_cast<unsigned char*>(vs_blob);
    vs_blob = nullptr; vs_size = 0;
    if (!m_vb) return false;

    // obtain the shaders from the shader library (opaque pixel shader)
    m_vs = sl->getVertexShader(L"BasicVertexShader.hlsl");
    m_ps = sl->getPixelShader(L"BasicPixelShader.hlsl");
    if (!m_vs || !m_ps) return false;

    CBC init = {};
    m_cb = rs->createConstantBuffer(&init, sizeof(CBC));
    if (!m_cb) return false;
    return true;
}

void Capsule::update(float dt)
{
    m_time += dt;
    if (!m_cb) return;

    CBC cb = {};
    cb.world = this->getWorldMatrix();
    cb.view = m_view;
    cb.projection = m_projection;
    cb.time = m_time;

    m_cb->update(GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get(), &cb);

    // Debug: log world translation and CB update
    Matrix4x4 world = this->getWorldMatrix();
    Vector3D tr = world.getTranslation();
    LOG_DEBUG("Capsule::update: world=(%.3f,%.3f,%.3f) time=%.3f cb=%p", tr.m_x, tr.m_y, tr.m_z, m_time, (void*)m_cb);
}

void Capsule::render()
{
    auto ctx = GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get();
    if (!ctx) {
        LOG_DEBUG("Capsule::render: no device context");
        return;
    }
    if (!m_vb || !m_ib || !m_vs || !m_ps) {
        LOG_DEBUG("Capsule::render: missing resources vb=%p ib=%p vs=%p ps=%p cb=%p instLayout=%p vs_inst=%p",
            (void*)m_vb, (void*)m_ib, (void*)m_vs, (void*)m_ps, (void*)m_cb, (void*)s_capsule_instancedLayout, (void*)s_capsule_vs_instanced);
        return;
    }

    // Prevent state bleed across different primitive types
    ctx->resetStateBindings();

    // Debug: log render call and resources
    Matrix4x4 world = this->getWorldMatrix();
    Vector3D tr = world.getTranslation();
    LOG_DEBUG("Capsule::render: world=(%.3f,%.3f,%.3f) vb=%p ib=%p vs=%p ps=%p cb=%p",
        tr.m_x, tr.m_y, tr.m_z, (void*)m_vb, (void*)m_ib, (void*)m_vs, (void*)m_ps, (void*)m_cb);

    // Debug: log instancing-related shared resources presence
    LOG_DEBUG("Capsule::render: instancedLayout=%p vs_instanced=%p instBuf=%p",
        (void*)s_capsule_instancedLayout, (void*)s_capsule_vs_instanced,
        (void*)(s_capsule_instanceBuffer ? s_capsule_instanceBuffer->getBuffer() : nullptr));

    ctx->setVertexShader(m_vs);
    ctx->setPixelShader(m_ps);
    ctx->setVertexBuffer(m_vb);
    ctx->setIndexBuffer(m_ib);

    // bind per-instance constant buffer for both VS and PS before drawing
    if (m_cb) {
        ctx->setConstantBuffer(m_vs, m_cb);
        ctx->setConstantBuffer(m_ps, m_cb);
    }

    ctx->drawIndexedTriangleList(m_ib->getSizeIndexList(), 0, 0);
}

void Capsule::setView(const Matrix4x4& v)
{
    m_view = v;
}

void Capsule::setProjection(const Matrix4x4& p)
{
    m_projection = p;
}

void Capsule::destroy()
{
    if (m_vb) { m_vb->release(); delete m_vb; m_vb = nullptr; }
    if (m_ib) { m_ib->release(); delete m_ib; m_ib = nullptr; }
    if (m_cb) { m_cb->release(); delete m_cb; m_cb = nullptr; }
    if (m_vs) { m_vs->release(); m_vs = nullptr; }
    if (m_ps) { m_ps->release(); m_ps = nullptr; }
}