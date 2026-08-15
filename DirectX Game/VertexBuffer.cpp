#include "VertexBuffer.h"
#include "RenderSystem.h"
#include "Debug.h"
#include "ReferenceManager.h"
#include <stdexcept>

VertexBuffer::VertexBuffer():m_system(nullptr), m_layout(0),m_buffer(0)
{
}

VertexBuffer::VertexBuffer(RenderSystem* system) : m_system(system), m_layout(0), m_buffer(0)
{
}

VertexBuffer::VertexBuffer(RenderSystem* system, void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader)
    : m_system(system), m_layout(0), m_buffer(0)
{
    if (!load(list_vertices, size_vertex, size_list, shader_byte_code, size_byte_shader)) {
        char buf[256];
        sprintf_s(buf, "VertexBuffer ctor: load failed (size_vertex=%u size_list=%u)", size_vertex, size_list);
        throw std::runtime_error(buf);
    }
}

bool VertexBuffer::load(void* list_vertices, UINT size_vertex, UINT size_list, void*shader_byte_code,UINT size_byte_shader)
{
    LOG("VertexBuffer::load size_vertex=%u size_list=%u bytecode=%p bytecodeSize=%u", size_vertex, size_list, shader_byte_code, size_byte_shader);

    if (m_buffer)m_buffer->Release();

    D3D11_BUFFER_DESC buff_desc = {};
    buff_desc.Usage = D3D11_USAGE_DEFAULT;
    buff_desc.ByteWidth = size_vertex * size_list;
    buff_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buff_desc.CPUAccessFlags = 0;
    buff_desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init_data = {};
    init_data.pSysMem = list_vertices;

    m_size_vertex = size_vertex;
    m_size_list = size_list;

    ID3D11Device* device = nullptr;
    if (m_system) device = m_system->getDevice();
    if (!device) {
        LOG("VertexBuffer::load - no D3D device available");
        return false;
    }
    if (!device) {
        LOG("VertexBuffer::load - no D3D device available");
        return false;
    }

    HRESULT hr = device->CreateBuffer(&buff_desc, &init_data, &m_buffer);
    if (FAILED(hr)) {
        LOG("VertexBuffer::load CreateBuffer failed HR=0x%08X", hr);
        return false;
    }

    ReferenceManager::acquire(m_buffer);

    UINT toLog = (size_list < 3) ? size_list : 3;
    for (UINT i = 0; i < toLog; ++i)
    {
        char* base = reinterpret_cast<char*>(list_vertices) + i * size_vertex;
        float* f = reinterpret_cast<float*>(base);
        float px = f[0], py = f[1], pz = f[2];
        float u = f[3], v = f[4];
        LOG("VertexBuffer::load vertex[%u] pos=(%f,%f,%f) tex=(%f,%f)", i, px, py, pz, u, v);
    }

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    UINT size_layout = ARRAYSIZE(layout);

    hr = device->CreateInputLayout(&layout[0], size_layout, shader_byte_code, size_byte_shader, &m_layout);
    if(FAILED(hr)) {
        LOG("VertexBuffer::load CreateInputLayout failed HR=0x%08X", hr);
        if (ReferenceManager::release(m_buffer)) m_buffer->Release();
        m_buffer = nullptr;
        return false;
    }

    LOG("VertexBuffer::load succeeded m_buffer=%p m_layout=%p", m_buffer, m_layout);
    return true;
}

UINT VertexBuffer::getSizeVertexList()
{
    return this->m_size_list;
}

bool VertexBuffer::release()
{
    if (m_layout)
    {
        m_layout->Release();
        m_layout = nullptr;
    }
    if (m_buffer)
    {
        if (ReferenceManager::release(m_buffer)) {
            m_buffer->Release();
        }
        m_buffer = nullptr;
    }
    return true;
}

VertexBuffer::~VertexBuffer()
{
    release();
}
