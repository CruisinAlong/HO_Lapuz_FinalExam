#include "PixelShader.h"
#include "PixelShader.h"
#include "RenderSystem.h"
#include "Debug.h"
#include <stdexcept>
#include "ReferenceManager.h"

PixelShader::PixelShader() : m_ps(nullptr), m_system(nullptr) { }

PixelShader::PixelShader(RenderSystem* system) : m_ps(nullptr), m_system(system) { }

PixelShader::PixelShader(RenderSystem* system, const void* shader_byte_code, size_t byte_code_size)
    : m_ps(nullptr), m_system(system)
{
    if (!init(shader_byte_code, byte_code_size)) {
        char buf[256];
        sprintf_s(buf, "PixelShader ctor: init failed (bytecode=%p size=%zu)", shader_byte_code, byte_code_size);
        throw std::runtime_error(buf);
    }
}

void PixelShader::release()
{
    if (m_ps)
    {
        if (ReferenceManager::release(m_ps)) {
            m_ps->Release();
        }
        m_ps = nullptr;
    }
}

PixelShader::~PixelShader()
{
    if (m_ps)
    {
        m_ps->Release();
        m_ps = nullptr;
    }
}

bool PixelShader::init(const void* shader_byte_code, size_t byte_code_size)
{
    ID3D11Device* device = nullptr;
    if (m_system) device = m_system->getDevice();
    if (!device) return false;
    if (!SUCCEEDED(device->CreatePixelShader(
            shader_byte_code, byte_code_size, nullptr, &m_ps)))
        return false;
    // register pixel shader in reference manager
    ReferenceManager::acquire(m_ps);
    return true;
}

