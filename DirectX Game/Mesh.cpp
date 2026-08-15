#define TINYOBJLOADER_IMPLEMENTATION
#include "Mesh.h"
#include "GraphicsEngine.h"
#include "RenderSystem.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShaderLibrary.h"
#include "ShaderNames.h"
#include "InstanceBuffer.h"
#include <tiny_obj_loader.h>
#include <locale>
#include <codecvt>
#include <string>
#include <cstring>

Mesh::Mesh(const std::wstring& full_path) : Resource(full_path)
{
    // Convert wide path to UTF-8 string
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string path = conv.to_bytes(full_path);

    std::vector<std::string> texnames;
    if (!loadObj(path, m_vertices, m_indices, texnames)) {
        // failed to load
        return;
    }

    // Create buffers via RenderSystem
    RenderSystem* rs = GraphicsEngine::getInstance()->getRenderSystem();
    if (!rs) return;

    // Compile vertex shader for mesh layout and create VB
    void* vs_blob = nullptr; size_t vs_size = 0;
    if (!rs->compileVertexShader(L"VertexMeshLayout.hlsl", "vsmain", &vs_blob, &vs_size)) return;

    m_vb = rs->createVertexBuffer(m_vertices.data(), sizeof(VertexMesh), static_cast<UINT>(m_vertices.size()), vs_blob, (UINT)vs_size);
    rs->releaseCompiledShader();

    m_ib = rs->createIndexBuffer(m_indices.data(), sizeof(unsigned int), static_cast<UINT>(m_indices.size()));


}

// Construct mesh from provided vertex/index arrays (procedural)
Mesh::Mesh(const std::vector<VertexMesh>& verts, const std::vector<unsigned int>& indices)
    : Resource(L"<procedural>")
{
    m_vertices = verts;
    m_indices = indices;

    RenderSystem* rs = GraphicsEngine::getInstance()->getRenderSystem();
    if (!rs) return;

    // Compile vertex shader for mesh layout and create VB
    void* vs_blob = nullptr; size_t vs_size = 0;
    if (!rs->compileVertexShader(L"VertexMeshLayout.hlsl", "vsmain", &vs_blob, &vs_size)) return;

    m_vb = rs->createVertexBuffer(m_vertices.data(), sizeof(VertexMesh), static_cast<UINT>(m_vertices.size()), vs_blob, (UINT)vs_size);
    rs->releaseCompiledShader();

    if (!m_indices.empty()) {
        m_ib = rs->createIndexBuffer(m_indices.data(), sizeof(unsigned int), static_cast<UINT>(m_indices.size()));
    }
}

Mesh::~Mesh()
{
    if (m_vb) { m_vb->release(); delete m_vb; m_vb = nullptr; }
    if (m_ib) { m_ib->release(); delete m_ib; m_ib = nullptr; }
    if (m_srv) { m_srv->Release(); m_srv = nullptr; }
    if (m_texture) { m_texture->Release(); m_texture = nullptr; }
    ReleaseInstanceBuffer();
}

bool Mesh::loadObj(const std::string& path, std::vector<VertexMesh>& outVertices, std::vector<unsigned int>& outIndices, std::vector<std::string>& outTexNames)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;


    std::string base_dir;
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) base_dir = path.substr(0, pos + 1);

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), base_dir.c_str(), true);
    if (!warn.empty()) {
        OutputDebugStringA(warn.c_str());
    }
    if (!err.empty()) {
        OutputDebugStringA(err.c_str());
    }
    if (!ret) return false;


    size_t totalFaces = 0;
    for (const auto& sh : shapes) totalFaces += sh.mesh.num_face_vertices.size();
    outVertices.reserve(totalFaces * 3);
    outIndices.reserve(totalFaces * 3);

    unsigned int index_offset = 0;
    for (size_t s = 0; s < shapes.size(); ++s)
    {
        const tinyobj::shape_t& shape = shapes[s];
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
        {
            int fv = shape.mesh.num_face_vertices[f];
            for (int v = 0; v < fv; ++v)
            {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                VertexMesh vm;
                if (idx.vertex_index >= 0) {
                    size_t vi = static_cast<size_t>(idx.vertex_index) * 3;
                    vm.position.m_x = attrib.vertices[vi + 0];
                    vm.position.m_y = attrib.vertices[vi + 1];
                    vm.position.m_z = attrib.vertices[vi + 2];
                } else {
                    vm.position = Vector3D(0.0f, 0.0f, 0.0f);
                }
                if (idx.texcoord_index >= 0) {
                    size_t ti = static_cast<size_t>(idx.texcoord_index) * 2;
                    vm.tex.m_x = attrib.texcoords[ti + 0];
                    vm.tex.m_y = attrib.texcoords[ti + 1];
                } else {
                    vm.tex = Vector2D(0.0f, 0.0f);
                }

                outVertices.push_back(vm);
                outIndices.push_back(static_cast<unsigned int>(outVertices.size() - 1));
            }
            index_offset += fv;
        }
    }

    // collect diffuse texture names
    for (const auto& m : materials) {
        if (!m.diffuse_texname.empty()) outTexNames.push_back(m.diffuse_texname);
    }

    return true;
}

bool Mesh::InitInstanceBuffer(RenderSystem* rs, UINT maxInstances)
{
    if (m_instanceBuffer) return true;
    if (!rs) return false;
    m_instanceBuffer = new InstanceBuffer();
    if (!m_instanceBuffer) return false;
    if (!m_instanceBuffer->create(rs, maxInstances, sizeof(Matrix4x4))) {
        delete m_instanceBuffer;
        m_instanceBuffer = nullptr;
        return false;
    }
    return true;
}

void Mesh::ReleaseInstanceBuffer()
{
    if (m_instanceBuffer) {
        m_instanceBuffer->release();
        delete m_instanceBuffer;
        m_instanceBuffer = nullptr;
    }
}

bool Mesh::UpdateInstanceBuffer(ID3D11DeviceContext* d3dContext, const Matrix4x4* matrices, UINT count)
{
    if (!m_instanceBuffer) return false;
    return m_instanceBuffer->update(d3dContext, matrices, count);
}

void Mesh::RenderInstanced(DeviceContext* ctx, UINT instanceCount)
{
    if (!ctx || !m_vb || !m_ib || instanceCount == 0 || !m_instanceBuffer) return;

    // Bind vertex/index for the mesh (slot 0)
    ctx->setVertexBuffer(m_vb);
    ctx->setIndexBuffer(m_ib);

    // Bind instance buffer to slot 1
    ctx->setInstanceBuffer(m_instanceBuffer->getBuffer(), m_instanceBuffer->getStride());

    // Issue instanced draw
    UINT indexCount = m_ib->getSizeIndexList();
    ctx->drawIndexedInstanced(indexCount, instanceCount, 0, 0);

    // Unbind instance buffer to keep cache consistent
    ctx->setInstanceBuffer(nullptr, 0);
}
