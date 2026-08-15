#include "GraphicsEngine.h"
#include "Plane.h"
#include "RenderSystem.h"
#include "VertexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ConstantBuffer.h"
#include "DeviceContext.h"
#include "Matrix4x4.h"
#include "Vector3D.h"
#include "InstanceBuffer.h"
#include "ShaderLibrary.h"
#include "ShaderNames.h"
#include "Debug.h"
#include <d3d11.h>

struct VertexP
{
	Vector3D position;
	Vector3D color0;
	Vector3D color1;
};

__declspec(align(16))
struct CBDataP
{
	Matrix4x4 world;
	Matrix4x4 view;
	Matrix4x4 projection;
	float time;
	float padding[3];
};


static VertexBuffer* s_plane_vb = nullptr;
static VertexShader* s_plane_vs = nullptr;
static PixelShader*  s_plane_ps = nullptr;
static ID3D11InputLayout* s_plane_instancedLayout = nullptr;
static bool s_plane_initialized = false;
static InstanceBuffer* s_plane_instanceBuffer = nullptr;
static VertexShader* s_plane_vs_instanced = nullptr;

Plane::Plane() : m_vb(nullptr), m_vs(nullptr), m_ps(nullptr), m_cb(nullptr), m_time(0.0f)
{
	m_view.SetIdentity();
	m_projection.SetIdentity();
}

Plane::~Plane()
{
	destroy();
}

bool Plane::create()
{
    GraphicsEngine* graphics = GraphicsEngine::getInstance();
	if (!graphics) return false;
	RenderSystem* rs = graphics->getRenderSystem();
	if (!rs) return false;

	const float planeY = 0.0f;
	const float halfExtent = 0.5f;

	VertexP list[] =
	{
		{ Vector3D(-halfExtent, planeY, -halfExtent), Vector3D(0.6f,0.6f,0.9f), Vector3D(0.6f,0.6f,0.9f) },
		{ Vector3D(-halfExtent, planeY,  halfExtent), Vector3D(0.6f,0.6f,0.9f), Vector3D(0.6f,0.6f,0.9f) },
		{ Vector3D( halfExtent, planeY, -halfExtent), Vector3D(0.6f,0.6f,0.9f), Vector3D(0.6f,0.6f,0.9f) },

		{ Vector3D(-halfExtent, planeY,  halfExtent), Vector3D(0.6f,0.6f,0.9f), Vector3D(0.6f,0.6f,0.9f) },
		{ Vector3D( halfExtent, planeY,  halfExtent), Vector3D(0.6f,0.6f,0.9f), Vector3D(0.6f,0.6f,0.9f) },
		{ Vector3D( halfExtent, planeY, -halfExtent), Vector3D(0.6f,0.6f,0.9f), Vector3D(0.6f,0.6f,0.9f) },
	};

	void* vs_blob = nullptr; size_t vs_size = 0;
    if (!rs->compileVertexShader(L"VertexShader.hlsl", "vsmain", &vs_blob, &vs_size)) return false;
	m_vs = rs->createVertexShader(vs_blob, vs_size);
	if (!m_vs) { rs->releaseCompiledShader(); return false; }

	m_vb = rs->createVertexBuffer(list, sizeof(VertexP), ARRAYSIZE(list), vs_blob, vs_size);
	if (!m_vb) { rs->releaseCompiledShader(); return false; }
	rs->releaseCompiledShader();

	void* ps_blob = nullptr; size_t ps_size = 0;
    if (!rs->compilePixelShader(L"PixelShader.hlsl", "psmain", &ps_blob, &ps_size)) return false;
	m_ps = rs->createPixelShader(ps_blob, ps_size);
	if (!m_ps) { rs->releaseCompiledShader(); return false; }
	rs->releaseCompiledShader();

    CBDataP init = {};
	m_cb = rs->createConstantBuffer(&init, sizeof(CBDataP));
	if (!m_cb) return false;
	return true;
}

void Plane::update(float dt)
{
	m_time += dt;
	if (!m_cb) return;

    CBDataP cb = {};
	// Use the GameObject world matrix so position/rotation/scale applied to the
	// Plane instance are respected when rendering.
	cb.world = getWorldMatrix();
	cb.view = m_view;
	cb.projection = m_projection;
	cb.time = m_time;

    m_cb->update(GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get(), &cb);
    auto ctx = GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get();
	// bind here is OK, but render resets bindings — ensure render rebinds as well
	ctx->setConstantBuffer(m_vs, m_cb);
	ctx->setConstantBuffer(m_ps, m_cb);
}

void Plane::render()
{
    auto ctx = GraphicsEngine::getInstance()->getRenderSystem()->getImmediateDeviceContext().get();
    if (!ctx) {
        LOG_DEBUG("Plane::render: no device context");
        return;
    }
    if (!m_vb || !m_vs || !m_ps) {
        LOG_DEBUG("Plane::render: missing resources vb=%p vs=%p ps=%p cb=%p instLayout=%p vs_inst=%p",
            (void*)m_vb, (void*)m_vs, (void*)m_ps, (void*)m_cb, (void*)s_plane_instancedLayout, (void*)s_plane_vs_instanced);
        return;
    }

	// Reset cached bindings to ensure plane-specific input layout and
	// constant buffers are bound correctly (prevents render state bleed).
	ctx->resetStateBindings();

	// Debug: log render call and resource pointers
	Matrix4x4 world = this->getWorldMatrix();
	Vector3D tr = world.getTranslation();
	LOG_DEBUG("Plane::render: world=(%.3f,%.3f,%.3f) vb=%p vs=%p ps=%p cb=%p instLayout=%p vs_inst=%p",
		tr.m_x, tr.m_y, tr.m_z, (void*)m_vb, (void*)m_vs, (void*)m_ps, (void*)m_cb, (void*)s_plane_instancedLayout, (void*)s_plane_vs_instanced);

	// Use the non-instanced VS variant for the non-instanced draw so the VS input signature matches the VB
	ctx->setVertexShader(m_vs);
	ctx->setPixelShader(m_ps);

	// setVertexBuffer will also bind the correct per-vertex input layout stored on the VB.
	ctx->setVertexBuffer(m_vb);

	// Do NOT override the input layout with the instanced layout for a non-instanced draw.
	// Overriding here causes the GPU to interpret per-vertex data as instance data and
	// results in invisible/misplaced geometry when instanced-layout exists.

    // Re-bind per-instance constant buffer after reset so the shader sees the correct world matrix.
	if (m_cb) {
		ctx->setConstantBuffer(m_vs, m_cb);
		ctx->setConstantBuffer(m_ps, m_cb);
	}

	ctx->drawTriangleList(6, 0);
}

void Plane::destroy()
{
	if (m_vb) { m_vb->release(); delete m_vb; m_vb = nullptr; }
	if (m_cb) { m_cb->release(); delete m_cb; m_cb = nullptr; }
	if (m_vs) { m_vs->release(); m_vs = nullptr; }
	if (m_ps) { m_ps->release(); m_ps = nullptr; }
}

bool Plane::InitSharedResources(RenderSystem* rs)
{
	if (s_plane_initialized) return true;
	if (!rs) return false;

	// Create a prototype plane and steal its vertex/shader resources
	Plane proto;
	if (!proto.create()) return false;

	s_plane_vb = proto.m_vb; proto.m_vb = nullptr;
	s_plane_vs = proto.m_vs; proto.m_vs = nullptr;
	s_plane_ps = proto.m_ps; proto.m_ps = nullptr;
	if (proto.m_cb) { proto.m_cb->release(); delete proto.m_cb; proto.m_cb = nullptr; }

	// Compile instanced vertex shader and create input layout
	void* vs_blob_inst = nullptr; size_t vs_size_inst = 0;
	if (rs->compileVertexShader(L"VertexShader.hlsl", "vsmain_instanced", &vs_blob_inst, &vs_size_inst)) {
		D3D11_INPUT_ELEMENT_DESC layoutInst[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// instance matrix rows -> TEXCOORD1..4
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
		ID3D11Device* device = rs->getDevice();
        if (device) {
			HRESULT hr = device->CreateInputLayout(layoutInst, ARRAYSIZE(layoutInst), vs_blob_inst, vs_size_inst, &s_plane_instancedLayout);
			if (FAILED(hr)) {
				LOG("Plane::InitSharedResources CreateInputLayout failed HR=0x%08X", hr);
				s_plane_instancedLayout = nullptr;
			}
		}
		s_plane_vs_instanced = rs->createVertexShader(vs_blob_inst, vs_size_inst);
		rs->releaseCompiledShader();
	}

	s_plane_initialized = true;
	return true;
}

void Plane::ReleaseSharedResources()
{
	s_plane_initialized = false;
	if (s_plane_vb) { s_plane_vb->release(); delete s_plane_vb; s_plane_vb = nullptr; }
	if (s_plane_vs) { s_plane_vs->release(); s_plane_vs = nullptr; }
	if (s_plane_ps) { s_plane_ps->release(); s_plane_ps = nullptr; }
    if (s_plane_instancedLayout) { s_plane_instancedLayout->Release(); s_plane_instancedLayout = nullptr; }
    if (s_plane_vs_instanced) { s_plane_vs_instanced->release(); s_plane_vs_instanced = nullptr; }
}

bool Plane::InitInstanceBuffer(RenderSystem* rs, UINT maxInstances)
{
	if (!rs) return false;
	if (s_plane_instanceBuffer) return true;
	s_plane_instanceBuffer = new InstanceBuffer();
	if (!s_plane_instanceBuffer) return false;
	if (!s_plane_instanceBuffer->create(rs, maxInstances, sizeof(Matrix4x4))) {
		delete s_plane_instanceBuffer;
		s_plane_instanceBuffer = nullptr;
		return false;
	}
	return true;
}

void Plane::ReleaseInstanceBuffer()
{
	if (s_plane_instanceBuffer) {
		s_plane_instanceBuffer->release();
		delete s_plane_instanceBuffer;
		s_plane_instanceBuffer = nullptr;
	}
}

bool Plane::UpdateInstanceBuffer(ID3D11DeviceContext* d3dCtx, const Matrix4x4* matrices, UINT count)
{
	if (!s_plane_instanceBuffer) return false;
	return s_plane_instanceBuffer->update(d3dCtx, matrices, count);
}

void Plane::RenderInstanced(DeviceContext* ctx, UINT instanceCount)
{
	if (!ctx || instanceCount == 0 || !s_plane_instanceBuffer) return;
	if (!s_plane_vb) return;


    if (s_plane_vs_instanced) ctx->setVertexShader(s_plane_vs_instanced);
	else if (s_plane_vs) ctx->setVertexShader(s_plane_vs);
	if (s_plane_ps) ctx->setPixelShader(s_plane_ps);
	ctx->setVertexBuffer(s_plane_vb);
	// If we have an instanced input layout, override the current layout so instance semantics are used
	if (s_plane_instancedLayout) ctx->setInputLayout(s_plane_instancedLayout);

	// Bind per-instance buffer to slot 1 and issue non-indexed instanced draw (6 vertices per plane)
	ctx->setInstanceBuffer(s_plane_instanceBuffer->getBuffer(), s_plane_instanceBuffer->getStride());
	ctx->drawInstanced(6, instanceCount, 0);
	ctx->setInstanceBuffer(nullptr, 0);
}

void Plane::setView(const Matrix4x4& v)
{
	m_view = v;
}

void Plane::setProjection(const Matrix4x4& p)
{
	m_projection = p;
}