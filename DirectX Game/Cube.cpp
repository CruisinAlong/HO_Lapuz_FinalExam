#include "Cube.h"
#include "GraphicsEngine.h"
#include "RenderSystem.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ConstantBuffer.h"
#include "DeviceContext.h"
#include "Matrix4x4.h"
#include <cstring>
#include "ShaderLibrary.h"
#include "ShaderNames.h"
#include "Texture.h"

#include "Vector2D.h"
#include "AComponent.h" 
#include "CorePrereqs.h"
#include "InstanceBuffer.h"

struct Vertex
{
    Vector3D position;
    Vector2D tex;
};

__declspec(align(16))
struct CBData
{
    Matrix4x4 world;
    Matrix4x4 view;
    Matrix4x4 projection;
    float time;
    float padding[3];
};

Cube::Cube() :
    m_vb(nullptr),
    m_ib(nullptr),
    m_vs(nullptr),
    m_ps(nullptr),
    m_cb(nullptr),
    m_time(0.0f)
{
    m_view.SetIdentity();
    m_projection.SetIdentity();
}

VertexBuffer* Cube::s_vb = nullptr;
IndexBuffer* Cube::s_ib = nullptr;
VertexShader* Cube::s_vs = nullptr;
PixelShader* Cube::s_ps = nullptr;
bool Cube::s_initialized = false;
InstanceBuffer* Cube::s_instanceBuffer = nullptr;

bool Cube::InitSharedResources(RenderSystem* rs)
{
    if (s_initialized) return true;
    if (!rs) return false;

    Vertex list[] =
    {
        { Vector3D(-0.5f, -0.5f,  0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D( 0.5f, -0.5f,  0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D( 0.5f,  0.5f,  0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D(-0.5f,  0.5f,  0.5f), Vector2D(0.0f, 0.0f) },

        { Vector3D( 0.5f, -0.5f, -0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D(-0.5f, -0.5f, -0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D(-0.5f,  0.5f, -0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D( 0.5f,  0.5f, -0.5f), Vector2D(0.0f, 0.0f) },

        { Vector3D(-0.5f, -0.5f, -0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D(-0.5f, -0.5f,  0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D(-0.5f,  0.5f,  0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D(-0.5f,  0.5f, -0.5f), Vector2D(0.0f, 0.0f) },

        { Vector3D( 0.5f, -0.5f,  0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D( 0.5f, -0.5f, -0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D( 0.5f,  0.5f, -0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D( 0.5f,  0.5f,  0.5f), Vector2D(0.0f, 0.0f) },

        { Vector3D(-0.5f,  0.5f,  0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D( 0.5f,  0.5f,  0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D( 0.5f,  0.5f, -0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D(-0.5f,  0.5f, -0.5f), Vector2D(0.0f, 0.0f) },

        { Vector3D(-0.5f, -0.5f, -0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D( 0.5f, -0.5f, -0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D( 0.5f, -0.5f,  0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D(-0.5f, -0.5f,  0.5f), Vector2D(0.0f, 0.0f) }
    };

    unsigned int index_list[] =
    {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };

    ShaderLibrary* lib = ShaderLibrary::getInstance();
    if (!lib) return false;

    void* vs_blob = nullptr; size_t vs_size = 0;
    lib->requestVertexShaderData(ShaderNames::BASIC_VS, &vs_blob, &vs_size);
    if (!vs_blob) return false;

    s_vs = lib->getVertexShader(ShaderNames::BASIC_VS);
    if (!s_vs) { delete[] reinterpret_cast<unsigned char*>(vs_blob); return false; }

    s_vb = rs->createVertexBuffer(list, sizeof(Vertex), ARRAYSIZE(list), vs_blob, vs_size);
    delete[] reinterpret_cast<unsigned char*>(vs_blob);
    if (!s_vb) return false;

    s_ib = rs->createIndexBuffer(index_list, sizeof(unsigned int), ARRAYSIZE(index_list));
    if (!s_ib) return false;

    s_ps = lib->getPixelShader(ShaderNames::BASIC_PS);
    if (!s_ps) return false;

    s_initialized = true;
    return true;
}

void Cube::ReleaseSharedResources()
{
    s_initialized = false;
    if (s_vb) { s_vb->release(); delete s_vb; s_vb = nullptr; }
    if (s_ib) { s_ib->release(); delete s_ib; s_ib = nullptr; }
    s_vs = nullptr;
    s_ps = nullptr;
}

bool Cube::InitInstanceBuffer(RenderSystem* rs, UINT maxInstances)
{
    if (s_instanceBuffer) return true;
    s_instanceBuffer = new InstanceBuffer();
    if (!s_instanceBuffer) return false;
    if (!s_instanceBuffer->create(rs, maxInstances, sizeof(Matrix4x4))) {
        delete s_instanceBuffer;
        s_instanceBuffer = nullptr;
        return false;
    }
    return true;
}

void Cube::ReleaseInstanceBuffer()
{
    if (s_instanceBuffer) {
        s_instanceBuffer->release();
        delete s_instanceBuffer;
        s_instanceBuffer = nullptr;
    }
}

Cube::~Cube()
{
    destroy();
}

bool Cube::create()
{
    GraphicsEngine* graphics = GraphicsEngine::getInstance();
    if (!graphics) return false;
    RenderSystem* rs = graphics->getRenderSystem();
    if (!rs) return false;

    if (s_initialized) {
        m_vb = s_vb;
        m_ib = s_ib;
        m_vs = s_vs;
        m_ps = s_ps;
    } else {
        if (!InitSharedResources(rs)) return false;
        m_vb = s_vb;
        m_ib = s_ib;
        m_vs = s_vs;
        m_ps = s_ps;
    }

    CBData init = {};
    m_cb = rs->createConstantBuffer(&init, sizeof(CBData));
    if (!m_cb) return false;

    return true;
}

void Cube::update(float dt)
{
    m_time += dt;

    if (m_cb)
    {
        CBData cb = {};
        cb.world = this->getWorldMatrix();
        cb.view = m_view;
        cb.projection = m_projection;
        cb.time = m_time;

        m_cb->update(GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get(), &cb);

        auto physComps = getComponentsOfType(AComponent::Physics);
        if (!physComps.empty()) {
            Vector3D trans = cb.world.getTranslation();
        }

    }
}

void Cube::render()
{
    DeviceContext* ctx = GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get();
    if (!ctx || !m_vb || !m_ib || !m_vs || !m_ps) return;

    ctx->resetStateBindings();

    ctx->setVertexShader(m_vs);
    ctx->setPixelShader(m_ps);
    ctx->setVertexBuffer(m_vb);
    ctx->setIndexBuffer(m_ib);

    if (m_texture) {
        ctx->setTexture(m_ps, m_texture->getSRV());
    }

    if (m_cb) {
        ctx->setConstantBuffer(m_vs, m_cb);
        ctx->setConstantBuffer(m_ps, m_cb);
    }

    ctx->drawIndexedTriangleList(m_ib->getSizeIndexList(), 0, 0);
}

void Cube::destroy()
{
    if (!m_externalBuffers) {
        if (m_vb && m_vb != s_vb) { m_vb->release(); delete m_vb; m_vb = nullptr; }
        if (m_ib && m_ib != s_ib) { m_ib->release(); delete m_ib; m_ib = nullptr; }
    } else {
        m_vb = nullptr;
        m_ib = nullptr;
    }

    if (m_cb) { m_cb->release(); delete m_cb; m_cb = nullptr; }

    m_vs = nullptr;
    m_ps = nullptr;
}

void Cube::setShaders(VertexShader* vs, PixelShader* ps)
{
    m_vs = vs;
    m_ps = ps;
}

void Cube::setBuffers(VertexBuffer* vb, IndexBuffer* ib)
{
    m_vb = vb;
    m_ib = ib;
    m_externalBuffers = true;
}

void Cube::setView(const Matrix4x4& v)
{
    m_view = v;
}

void Cube::setProjection(const Matrix4x4& p)
{
    m_projection = p;
}

bool Cube::UpdateInstanceBuffer(ID3D11DeviceContext* d3dContext, const Matrix4x4* matrices, UINT count)
{
    if (!s_instanceBuffer) return false;
    return s_instanceBuffer->update(d3dContext, matrices, count);
}

void Cube::RenderInstanced(ID3D11DeviceContext* d3dContext, UINT instanceCount)
{
    if (!d3dContext || !s_vb || !s_ib) return;

}