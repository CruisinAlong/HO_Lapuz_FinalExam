#pragma once
#include "Resource.h"
#include "Prerequisites.h"
#include "Vector3D.h"
#include "Vector2D.h"
#include "Matrix4x4.h"
#include <vector>

class VertexBuffer;
class IndexBuffer;

struct VertexMesh
{
    Vector3D position;
    Vector2D tex;
};

class Mesh : public Resource
{
public:
    Mesh(const std::wstring& full_path);
    // Construct a mesh from raw vertex/index data (procedural)
    Mesh(const std::vector<VertexMesh>& verts, const std::vector<unsigned int>& indices);
    ~Mesh();

    VertexBuffer* getVertexBuffer() { return m_vb; }
    IndexBuffer* getIndexBuffer() { return m_ib; }
    ID3D11ShaderResourceView* getSRV() { return m_srv; }

private:
    bool loadObj(const std::string& path, std::vector<VertexMesh>& outVertices, std::vector<unsigned int>& outIndices, std::vector<std::string>& outTexNames);

private:
    VertexBuffer* m_vb = nullptr;
    IndexBuffer* m_ib = nullptr;
    ID3D11ShaderResourceView* m_srv = nullptr;
    ID3D11Resource* m_texture = nullptr;
    class InstanceBuffer* m_instanceBuffer = nullptr;

    std::vector<VertexMesh> m_vertices;
    std::vector<unsigned int> m_indices;

public:
    bool InitInstanceBuffer(class RenderSystem* rs, UINT maxInstances);
    void ReleaseInstanceBuffer();
    bool UpdateInstanceBuffer(ID3D11DeviceContext* d3dContext, const class Matrix4x4* matrices, UINT count);
    void RenderInstanced(class DeviceContext* ctx, UINT instanceCount);
};
