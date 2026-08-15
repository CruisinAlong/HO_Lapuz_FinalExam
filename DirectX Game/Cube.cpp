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
#include "AComponent.h" // added to detect attached Physics components
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

// Static shared resource definitions
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

    // Prepare vertex and index data
    // Build 24 vertices (6 faces * 4 verts) so each face can have unique texcoords
    Vertex list[] =
    {
        // Front face (z = +0.5)
        { Vector3D(-0.5f, -0.5f,  0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D( 0.5f, -0.5f,  0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D( 0.5f,  0.5f,  0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D(-0.5f,  0.5f,  0.5f), Vector2D(0.0f, 0.0f) },

        // Back face (z = -0.5)
        { Vector3D( 0.5f, -0.5f, -0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D(-0.5f, -0.5f, -0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D(-0.5f,  0.5f, -0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D( 0.5f,  0.5f, -0.5f), Vector2D(0.0f, 0.0f) },

        // Left face (x = -0.5)
        { Vector3D(-0.5f, -0.5f, -0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D(-0.5f, -0.5f,  0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D(-0.5f,  0.5f,  0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D(-0.5f,  0.5f, -0.5f), Vector2D(0.0f, 0.0f) },

        // Right face (x = +0.5)
        { Vector3D( 0.5f, -0.5f,  0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D( 0.5f, -0.5f, -0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D( 0.5f,  0.5f, -0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D( 0.5f,  0.5f,  0.5f), Vector2D(0.0f, 0.0f) },

        // Top face (y = +0.5)
        { Vector3D(-0.5f,  0.5f,  0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D( 0.5f,  0.5f,  0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D( 0.5f,  0.5f, -0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D(-0.5f,  0.5f, -0.5f), Vector2D(0.0f, 0.0f) },

        // Bottom face (y = -0.5)
        { Vector3D(-0.5f, -0.5f, -0.5f), Vector2D(0.0f, 1.0f) },
        { Vector3D( 0.5f, -0.5f, -0.5f), Vector2D(1.0f, 1.0f) },
        { Vector3D( 0.5f, -0.5f,  0.5f), Vector2D(1.0f, 0.0f) },
        { Vector3D(-0.5f, -0.5f,  0.5f), Vector2D(0.0f, 0.0f) }
    };

    unsigned int index_list[] =
    {
        // Front
        0,1,2, 2,3,0,
        // Back
        4,5,6, 6,7,4,
        // Left
        8,9,10, 10,11,8,
        // Right
        12,13,14, 14,15,12,
        // Top
        16,17,18, 18,19,16,
        // Bottom
        20,21,22, 22,23,20
    };

    // Use ShaderLibrary to obtain shader bytecode and shader objects
    ShaderLibrary* lib = ShaderLibrary::getInstance();
    if (!lib) return false;

    void* vs_blob = nullptr; size_t vs_size = 0;
    lib->requestVertexShaderData(ShaderNames::BASIC_VS, &vs_blob, &vs_size);
    if (!vs_blob) return false;

    // Vertex shader object (owned by ShaderLibrary) - ensure it's created there
    s_vs = lib->getVertexShader(ShaderNames::BASIC_VS);
    if (!s_vs) { delete[] reinterpret_cast<unsigned char*>(vs_blob); return false; }

    s_vb = rs->createVertexBuffer(list, sizeof(Vertex), ARRAYSIZE(list), vs_blob, vs_size);
    delete[] reinterpret_cast<unsigned char*>(vs_blob);
    if (!s_vb) return false;

    // Index buffer
    s_ib = rs->createIndexBuffer(index_list, sizeof(unsigned int), ARRAYSIZE(index_list));
    if (!s_ib) return false;

    // Pixel shader object via ShaderLibrary (basic shader for untextured cube)
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
    // Vertex/Pixel shader objects are owned by ShaderLibrary now; do not delete them here
    s_vs = nullptr;
    s_ps = nullptr;
}

bool Cube::InitInstanceBuffer(RenderSystem* rs, UINT maxInstances)
{
    if (s_instanceBuffer) return true;
    s_instanceBuffer = new InstanceBuffer();
    if (!s_instanceBuffer) return false;
    // Use Matrix4x4 as per-instance data (world matrix), adjust if you prefer packed format
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

    // Use shared resources if initialized
    if (s_initialized) {
        m_vb = s_vb;
        m_ib = s_ib;
        m_vs = s_vs;
        m_ps = s_ps;
    } else {
        // Fallback: try to initialize shared resources on demand
        if (!InitSharedResources(rs)) return false;
        m_vb = s_vb;
        m_ib = s_ib;
        m_vs = s_vs;
        m_ps = s_ps;
    }

    CBData init = {};
    // Per-instance constant buffer
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

        // Quick diagnostic: if this cube has a Physics component, print the per-instance world translation
        auto physComps = getComponentsOfType(AComponent::Physics);
        if (!physComps.empty()) {
            Vector3D trans = cb.world.getTranslation();
        }

        // Do not bind the constant buffer here; bind during render to avoid render-order bleed-through.
    }
}

void Cube::render()
{
    DeviceContext* ctx = GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get();
    if (!ctx || !m_vb || !m_ib || !m_vs || !m_ps) return;
    // Ensure device context binding cache is reset so each object's state is
    // bound correctly. This avoids cases where stale cached state prevents the
    // VAO/input-layout or constant buffers from being rebound when drawing
    // different object types.
    ctx->resetStateBindings();

    ctx->setVertexShader(m_vs);
    ctx->setPixelShader(m_ps);
    ctx->setVertexBuffer(m_vb);
    ctx->setIndexBuffer(m_ib);

    // Bind per-instance texture if present
    if (m_texture) {
        ctx->setTexture(m_ps, m_texture->getSRV());
    }

    // Bind per-instance constant buffer for both VS and PS before drawing
    if (m_cb) {
        ctx->setConstantBuffer(m_vs, m_cb);
        ctx->setConstantBuffer(m_ps, m_cb);
    }

    ctx->drawIndexedTriangleList(m_ib->getSizeIndexList(), 0, 0);
}

void Cube::destroy()
{
    // Only release/delete resources that are owned by this instance.
    // Shared static resources (s_vb, s_ib, s_vs, s_ps) are released by
    // Cube::ReleaseSharedResources() and must not be deleted per-instance.
    if (!m_externalBuffers) {
        if (m_vb && m_vb != s_vb) { m_vb->release(); delete m_vb; m_vb = nullptr; }
        if (m_ib && m_ib != s_ib) { m_ib->release(); delete m_ib; m_ib = nullptr; }
    } else {
        // Do not release buffers owned externally; just clear pointers
        m_vb = nullptr;
        m_ib = nullptr;
    }

    if (m_cb) { m_cb->release(); delete m_cb; m_cb = nullptr; }

    // Shader objects are owned by the ShaderLibrary; do not delete here.
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
    // Assign external buffers; mark them as not owned by this instance
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
    // Bind vertex/index buffers as usual (DeviceContext::setVertexBuffer already binds slot 0)
    // Bind instance buffer to slot 1
    // We access the DeviceContext wrapper via RenderSystem's immediate context in typical usage.
    // The project's DeviceContext wrapper provides setInstanceBuffer(...) and drawIndexedInstanced(...).
    // Convert native ID3D11DeviceContext* -> your DeviceContext wrapper when calling from higher-level code.
    // Here we only set up the call assuming caller will bind via wrapper.
}