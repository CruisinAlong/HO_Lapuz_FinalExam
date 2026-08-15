#include "ShaderLibrary.h"
#include "RenderSystem.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include <cstring>
#include <stdexcept>

ShaderLibrary* ShaderLibrary::sharedInstance = nullptr;

ShaderLibrary::ShaderLibrary(RenderSystem* rs) : m_rs(rs) {}
ShaderLibrary::~ShaderLibrary()
{
    for (auto& kv : m_vertexShaders) {
        if (kv.second) { kv.second->release(); delete kv.second; }
    }
    m_vertexShaders.clear();
    for (auto& kv : m_pixelShaders) {
        if (kv.second) { kv.second->release(); delete kv.second; }
    }
    m_pixelShaders.clear();
}

ShaderLibrary* ShaderLibrary::getInstance()
{
    return sharedInstance;
}

void ShaderLibrary::initialize(RenderSystem* rs)
{
    if (!sharedInstance) sharedInstance = new ShaderLibrary(rs);
}

void ShaderLibrary::destroy()
{
    if (sharedInstance) {
        delete sharedInstance;
        sharedInstance = nullptr;
    }
}

void ShaderLibrary::requestVertexShaderData(const String& filename, void** shaderByteCode, size_t* sizeShader)
{
    if (!m_rs) { *shaderByteCode = nullptr; *sizeShader = 0; return; }
    void* blob = nullptr; size_t sz = 0;
    if (!m_rs->compileVertexShader(filename.c_str(), "vsmain", &blob, &sz)) { *shaderByteCode = nullptr; *sizeShader = 0; return; }
    if (!blob || sz == 0) { m_rs->releaseCompiledShader(); *shaderByteCode = nullptr; *sizeShader = 0; return; }
    unsigned char* copy = new unsigned char[sz];
    memcpy(copy, blob, sz);
    m_rs->releaseCompiledShader();
    *shaderByteCode = copy;
    *sizeShader = sz;
}

void ShaderLibrary::requestPixelShaderData(const String& filename, void** shaderByteCode, size_t* sizeShader)
{
    if (!m_rs) { *shaderByteCode = nullptr; *sizeShader = 0; return; }
    void* blob = nullptr; size_t sz = 0;
    if (!m_rs->compilePixelShader(filename.c_str(), "psmain", &blob, &sz)) { *shaderByteCode = nullptr; *sizeShader = 0; return; }
    if (!blob || sz == 0) { m_rs->releaseCompiledShader(); *shaderByteCode = nullptr; *sizeShader = 0; return; }
    unsigned char* copy = new unsigned char[sz];
    memcpy(copy, blob, sz);
    m_rs->releaseCompiledShader();
    *shaderByteCode = copy;
    *sizeShader = sz;
}

VertexShader* ShaderLibrary::getVertexShader(const String& filename)
{
    auto it = m_vertexShaders.find(filename);
    if (it != m_vertexShaders.end()) return it->second;
    if (!m_rs) return nullptr;
    void* blob = nullptr; size_t sz = 0;
    requestVertexShaderData(filename, &blob, &sz);
    if (!blob) return nullptr;
    VertexShader* vs = nullptr;
    try {
        vs = m_rs->createVertexShader(blob, sz);
    }
    catch (...) { vs = nullptr; }
    delete[] reinterpret_cast<unsigned char*>(blob);
    if (vs) m_vertexShaders.emplace(filename, vs);
    return vs;
}

PixelShader* ShaderLibrary::getPixelShader(const String& filename)
{
    auto it = m_pixelShaders.find(filename);
    if (it != m_pixelShaders.end()) return it->second;
    if (!m_rs) return nullptr;
    void* blob = nullptr; size_t sz = 0;
    requestPixelShaderData(filename, &blob, &sz);
    if (!blob) return nullptr;
    PixelShader* ps = nullptr;
    try {
        ps = m_rs->createPixelShader(blob, sz);
    }
    catch (...) { ps = nullptr; }
    delete[] reinterpret_cast<unsigned char*>(blob);
    if (ps) m_pixelShaders.emplace(filename, ps);
    return ps;
}
