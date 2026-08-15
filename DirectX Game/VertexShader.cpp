#include "VertexShader.h"
#include "VertexShader.h"
#include "RenderSystem.h"
#include "Debug.h"
#include <stdexcept>
#include "ReferenceManager.h"

VertexShader::VertexShader() : m_vs(nullptr), m_system(nullptr) { }

VertexShader::VertexShader(RenderSystem* system) : m_vs(nullptr), m_system(system) { }

VertexShader::VertexShader(RenderSystem* system, const void* shader_byte_code, size_t byte_code_size)
    : m_vs(nullptr), m_system(system)
{
    if (!init(shader_byte_code, byte_code_size)) {
        char buf[256];
        sprintf_s(buf, "VertexShader ctor: init failed (bytecode=%p size=%zu)", shader_byte_code, byte_code_size);
        throw std::runtime_error(buf);
    }
}

void VertexShader::release()
{
    if (m_vs)
    {
        if (ReferenceManager::release(m_vs)) {
            m_vs->Release();
        }
        m_vs = nullptr;
    }
}

VertexShader::~VertexShader()
{
    if (m_vs)
    {
        m_vs->Release();
        m_vs = nullptr;
    }
}

bool VertexShader::init(const void* shader_byte_code, size_t byte_code_size)
{
    ID3D11Device* device = nullptr;
    if (m_system) device = m_system->getDevice();
    if (!device) return false;
    if (!SUCCEEDED(device->CreateVertexShader(
            shader_byte_code, byte_code_size, nullptr, &m_vs)))
        return false;
    ReferenceManager::acquire(m_vs);
    return true;
}
