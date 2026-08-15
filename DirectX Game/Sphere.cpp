#include "Sphere.h"
#include "GraphicsEngine.h"
#include "RenderSystem.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ConstantBuffer.h"
#include "DeviceContext.h"
#include <d3d11.h>
#include "Matrix4x4.h"
#include "Vector3D.h"
#include "InstanceBuffer.h"
#include "Debug.h"
#include <vector>
#include <cmath>

struct VertexS
{
	Vector3D position;
	Vector3D color0;
	Vector3D color1;
};

__declspec(align(16))
struct CBDataS
{
	Matrix4x4 world;
	Matrix4x4 view;
	Matrix4x4 projection;
	float time;
	float padding[3];
};

Sphere::Sphere() : m_vb(nullptr), m_ib(nullptr), m_vs(nullptr), m_ps(nullptr), m_cb(nullptr), m_time(0.0f)
{
	m_view.SetIdentity();
	m_projection.SetIdentity();
}

Sphere::~Sphere()
{
	destroy();
}

bool Sphere::create(int segments, int rings)
{
	GraphicsEngine* graphics = GraphicsEngine::getInstance();
	if (!graphics) return false;

	const float radius = 0.5f;
	const float PI = 3.14159265358979323846f;

	std::vector<VertexS> vertices;
	std::vector<unsigned int> indices;

	for (int ring = 0; ring <= rings; ++ring)
	{
		float phi = PI * (float)ring / (float)rings;
		for (int seg = 0; seg <= segments; ++seg)
		{
			float theta = 2.0f * PI * (float)seg / (float)segments;

			float x = radius * std::sinf(phi) * std::cosf(theta);
			float y = radius * std::cosf(phi);
			float z = radius * std::sinf(phi) * std::sinf(theta);

			Vector3D color(0.8f, 0.3f, 0.5f);
			vertices.push_back({ Vector3D(x, y, z), color, color });
		}
	}

	for (int ring = 0; ring < rings; ++ring)
	{
		for (int seg = 0; seg < segments; ++seg)
		{
			int current = ring * (segments + 1) + seg;
			int next = current + segments + 1;

			indices.push_back(current);
			indices.push_back(next);
			indices.push_back(current + 1);

			indices.push_back(current + 1);
			indices.push_back(next);
			indices.push_back(next + 1);
		}
	}

    RenderSystem* rs = graphics->getRenderSystem();
	if (!rs) return false;

	m_ib = rs->createIndexBuffer(indices.data(), sizeof(unsigned int), static_cast<UINT>(indices.size()));
	if (!m_ib) return false;

	void* vs_blob = nullptr; size_t vs_size = 0;
    if (!rs->compileVertexShader(L"VertexShader.hlsl", "vsmain", &vs_blob, &vs_size)) return false;
	m_vs = rs->createVertexShader(vs_blob, vs_size);
	if (!m_vs) { rs->releaseCompiledShader(); return false; }

    m_vb = rs->createVertexBuffer(const_cast<void*>(reinterpret_cast<const void*>(vertices.data())), sizeof(VertexS), static_cast<UINT>(vertices.size()), vs_blob, vs_size);
	if (!m_vb) { rs->releaseCompiledShader(); return false; }
	rs->releaseCompiledShader();

    void* ps_blob = nullptr; size_t ps_size = 0;
	if (!rs->compilePixelShader(L"PixelShader.hlsl", "psmain", &ps_blob, &ps_size)) return false;
	m_ps = rs->createPixelShader(ps_blob, ps_size);
	if (!m_ps) { rs->releaseCompiledShader(); return false; }
	rs->releaseCompiledShader();

	CBDataS init = {};
    m_cb = rs->createConstantBuffer(&init, sizeof(CBDataS));
	if (!m_cb) return false;
	return true;
}

void Sphere::update(float dt)
{
	m_time += dt;
	if (!m_cb) return;

	CBDataS cb = {};
	cb.world = this->getWorldMatrix();
	cb.view = m_view;
	cb.projection = m_projection;
	cb.time = m_time;

    m_cb->update(GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get(), &cb);

	Matrix4x4 world = this->getWorldMatrix();
	Vector3D tr = world.getTranslation();
	LOG_DEBUG("Sphere::update: world=(%.3f,%.3f,%.3f) time=%.3f cb=%p", tr.m_x, tr.m_y, tr.m_z, m_time, (void*)m_cb);
}

void Sphere::render()
{
    auto ctx = GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get();
    if (!ctx) {
        LOG_DEBUG("Sphere::render: no device context");
        return;
    }
    if (!m_vb || !m_ib || !m_vs || !m_ps) {
        LOG_DEBUG("Sphere::render: missing resources vb=%p ib=%p vs=%p ps=%p cb=%p",
            (void*)m_vb, (void*)m_ib, (void*)m_vs, (void*)m_ps, (void*)m_cb);
        return;
    }

    ctx->resetStateBindings();

	Matrix4x4 world = this->getWorldMatrix();
	Vector3D tr = world.getTranslation();
	LOG_DEBUG("Sphere::render: world=(%.3f,%.3f,%.3f) vb=%p ib=%p vs=%p ps=%p cb=%p",
		tr.m_x, tr.m_y, tr.m_z, (void*)m_vb, (void*)m_ib, (void*)m_vs, (void*)m_ps, (void*)m_cb);

    ctx->setVertexShader(m_vs);
    ctx->setPixelShader(m_ps);
    ctx->setVertexBuffer(m_vb);
    ctx->setIndexBuffer(m_ib);

	if (m_cb) {
		ctx->setConstantBuffer(m_vs, m_cb);
		ctx->setConstantBuffer(m_ps, m_cb);
	}

	ctx->drawIndexedTriangleList(m_ib->getSizeIndexList(), 0, 0);
}

void Sphere::destroy()
{
	if (m_vb) { m_vb->release(); delete m_vb; m_vb = nullptr; }
	if (m_ib) { m_ib->release(); delete m_ib; m_ib = nullptr; }
	if (m_cb) { m_cb->release(); delete m_cb; m_cb = nullptr; }
	if (m_vs) { m_vs->release(); m_vs = nullptr; }
	if (m_ps) { m_ps->release(); m_ps = nullptr; }
}

void Sphere::setView(const Matrix4x4& v)
{
	m_view = v;
}

void Sphere::setProjection(const Matrix4x4& p)
{
	m_projection = p;
}


static VertexBuffer* s_sphere_vb = nullptr;
static IndexBuffer*  s_sphere_ib = nullptr;
static VertexShader* s_sphere_vs = nullptr;
static PixelShader*  s_sphere_ps = nullptr;
static ID3D11InputLayout* s_sphere_instancedLayout = nullptr;
static VertexShader* s_sphere_vs_instanced = nullptr;
static bool s_sphere_initialized = false;
static InstanceBuffer* s_sphere_instanceBuffer = nullptr;

bool Sphere::InitSharedResources(RenderSystem* rs, int segments, int rings)
{
    if (s_sphere_initialized) return true;
    if (!rs) return false;

    Sphere proto;
    if (!proto.create(segments, rings)) return false;

    s_sphere_vb = proto.m_vb; proto.m_vb = nullptr;
    s_sphere_ib = proto.m_ib; proto.m_ib = nullptr;
    s_sphere_vs = proto.m_vs; proto.m_vs = nullptr;
    s_sphere_ps = proto.m_ps; proto.m_ps = nullptr;
    if (proto.m_cb) { proto.m_cb->release(); delete proto.m_cb; proto.m_cb = nullptr; }

	void* vs_blob_inst = nullptr; size_t vs_size_inst = 0;
	if (rs->compileVertexShader(L"VertexShader.hlsl", "vsmain_instanced", &vs_blob_inst, &vs_size_inst)) {
		D3D11_INPUT_ELEMENT_DESC layoutInst[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
		ID3D11Device* device = rs->getDevice();
        if (device) {
			HRESULT hr = device->CreateInputLayout(layoutInst, ARRAYSIZE(layoutInst), vs_blob_inst, vs_size_inst, &s_sphere_instancedLayout);
			if (FAILED(hr)) {
				LOG("Sphere::InitSharedResources CreateInputLayout failed HR=0x%08X", hr);
				s_sphere_instancedLayout = nullptr;
			}
		}
		s_sphere_vs_instanced = rs->createVertexShader(vs_blob_inst, vs_size_inst);
		rs->releaseCompiledShader();
	}

    s_sphere_initialized = true;
    return true;
}

void Sphere::ReleaseSharedResources()
{
    s_sphere_initialized = false;
    if (s_sphere_vb) { s_sphere_vb->release(); delete s_sphere_vb; s_sphere_vb = nullptr; }
    if (s_sphere_ib) { s_sphere_ib->release(); delete s_sphere_ib; s_sphere_ib = nullptr; }
    if (s_sphere_vs) { s_sphere_vs->release(); s_sphere_vs = nullptr; }
    if (s_sphere_ps) { s_sphere_ps->release(); s_sphere_ps = nullptr; }
    if (s_sphere_instancedLayout) { s_sphere_instancedLayout->Release(); s_sphere_instancedLayout = nullptr; }
	if (s_sphere_vs_instanced) { s_sphere_vs_instanced->release(); s_sphere_vs_instanced = nullptr; }
}

bool Sphere::InitInstanceBuffer(RenderSystem* rs, unsigned int maxInstances)
{
    if (!rs) return false;
    if (s_sphere_instanceBuffer) return true;
    s_sphere_instanceBuffer = new InstanceBuffer();
    if (!s_sphere_instanceBuffer) return false;
    if (!s_sphere_instanceBuffer->create(rs, maxInstances, sizeof(Matrix4x4))) {
        delete s_sphere_instanceBuffer;
        s_sphere_instanceBuffer = nullptr;
        return false;
    }
    return true;
}

void Sphere::ReleaseInstanceBuffer()
{
    if (s_sphere_instanceBuffer) {
        s_sphere_instanceBuffer->release();
        delete s_sphere_instanceBuffer;
        s_sphere_instanceBuffer = nullptr;
    }
}

bool Sphere::UpdateInstanceBuffer(ID3D11DeviceContext* d3dCtx, const Matrix4x4* matrices, UINT count)
{
    if (!s_sphere_instanceBuffer) return false;
    return s_sphere_instanceBuffer->update(d3dCtx, matrices, count);
}

void Sphere::RenderInstanced(DeviceContext* ctx, UINT instanceCount)
{
    if (!ctx || instanceCount == 0 || !s_sphere_instanceBuffer) return;
    if (!s_sphere_vb || !s_sphere_ib) return;

    if (s_sphere_vs) ctx->setVertexShader(s_sphere_vs);
	if (s_sphere_vs_instanced) ctx->setVertexShader(s_sphere_vs_instanced);
	else if (s_sphere_vs) ctx->setVertexShader(s_sphere_vs);
	if (s_sphere_ps) ctx->setPixelShader(s_sphere_ps);
    ctx->setVertexBuffer(s_sphere_vb);
	if (s_sphere_instancedLayout) ctx->setInputLayout(s_sphere_instancedLayout);
    ctx->setIndexBuffer(s_sphere_ib);

    ctx->setInstanceBuffer(s_sphere_instanceBuffer->getBuffer(), s_sphere_instanceBuffer->getStride());
    ctx->drawIndexedInstanced(s_sphere_ib->getSizeIndexList(), instanceCount, 0, 0);
    ctx->setInstanceBuffer(nullptr, 0);
}