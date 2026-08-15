#include "MeshManager.h"
#include "Mesh.h"
#include <Windows.h>

MeshManager::MeshManager() {}
MeshManager::~MeshManager() {}

Resource* MeshManager::createResourceFromFileConcrete(const std::wstring& absolute_path)
{
    try {
        Mesh* mesh = new Mesh(absolute_path);
        return mesh;
    }
    catch (...) {
        return nullptr;
    }

}

MeshPtr MeshManager::createCubeMesh()
{
    std::vector<VertexMesh> verts;
    std::vector<unsigned int> idx;
    verts.reserve(24);
    idx.reserve(36);

    const float hx = 0.5f, hy = 0.5f, hz = 0.5f;

    verts.push_back({ Vector3D(hx, -hy, -hz), Vector2D(0.0f, 1.0f) });
    verts.push_back({ Vector3D(hx,  hy, -hz), Vector2D(0.0f, 0.0f) });
    verts.push_back({ Vector3D(hx,  hy,  hz), Vector2D(1.0f, 0.0f) });
    verts.push_back({ Vector3D(hx, -hy,  hz), Vector2D(1.0f, 1.0f) });

    verts.push_back({ Vector3D(-hx, -hy,  hz), Vector2D(0.0f, 1.0f) });
    verts.push_back({ Vector3D(-hx,  hy,  hz), Vector2D(0.0f, 0.0f) });
    verts.push_back({ Vector3D(-hx,  hy, -hz), Vector2D(1.0f, 0.0f) });
    verts.push_back({ Vector3D(-hx, -hy, -hz), Vector2D(1.0f, 1.0f) });

    verts.push_back({ Vector3D(-hx, hy, -hz), Vector2D(0.0f, 1.0f) });
    verts.push_back({ Vector3D(-hx, hy,  hz), Vector2D(0.0f, 0.0f) });
    verts.push_back({ Vector3D( hx, hy,  hz), Vector2D(1.0f, 0.0f) });
    verts.push_back({ Vector3D( hx, hy, -hz), Vector2D(1.0f, 1.0f) });

    verts.push_back({ Vector3D(-hx, -hy,  hz), Vector2D(0.0f, 1.0f) });
    verts.push_back({ Vector3D(-hx, -hy, -hz), Vector2D(0.0f, 0.0f) });
    verts.push_back({ Vector3D( hx, -hy, -hz), Vector2D(1.0f, 0.0f) });
    verts.push_back({ Vector3D( hx, -hy,  hz), Vector2D(1.0f, 1.0f) });

    verts.push_back({ Vector3D(-hx, -hy, -hz), Vector2D(0.0f, 1.0f) });
    verts.push_back({ Vector3D(-hx,  hy, -hz), Vector2D(0.0f, 0.0f) });
    verts.push_back({ Vector3D( hx,  hy, -hz), Vector2D(1.0f, 0.0f) });
    verts.push_back({ Vector3D( hx, -hy, -hz), Vector2D(1.0f, 1.0f) });

    verts.push_back({ Vector3D( hx, -hy, hz), Vector2D(0.0f, 1.0f) });
    verts.push_back({ Vector3D( hx,  hy, hz), Vector2D(0.0f, 0.0f) });
    verts.push_back({ Vector3D(-hx,  hy, hz), Vector2D(1.0f, 0.0f) });
    verts.push_back({ Vector3D(-hx, -hy, hz), Vector2D(1.0f, 1.0f) });

    for (unsigned int f = 0; f < 6; ++f) {
        unsigned int base = f * 4;
        idx.push_back(base + 0);
        idx.push_back(base + 1);
        idx.push_back(base + 2);
        idx.push_back(base + 0);
        idx.push_back(base + 2);
        idx.push_back(base + 3);
    }

    Mesh* m = new Mesh(verts, idx);
    return MeshPtr(m);
}

MeshPtr MeshManager::createPlaneMesh()
{
    std::vector<VertexMesh> verts(4);
    std::vector<unsigned int> idx = {0,1,2, 0,2,3};

    float hs = 0.5f;
    verts[0].position = Vector3D(-hs, 0.0f, -hs); verts[0].tex = Vector2D(0.0f, 1.0f);
    verts[1].position = Vector3D(-hs, 0.0f, hs); verts[1].tex = Vector2D(0.0f, 0.0f);
    verts[2].position = Vector3D(hs, 0.0f, hs); verts[2].tex = Vector2D(1.0f, 0.0f);
    verts[3].position = Vector3D(hs, 0.0f, -hs); verts[3].tex = Vector2D(1.0f, 1.0f);

    Mesh* m = new Mesh(verts, idx);
    return MeshPtr(m);
}

MeshPtr MeshManager::createMeshFromFile(const std::wstring& file_path)
{
    if (file_path.empty()) return MeshPtr();

    wchar_t buf[MAX_PATH];
    DWORD len = ::GetFullPathNameW(file_path.c_str(), MAX_PATH, buf, nullptr);
    std::wstring key;
    if (len == 0) key = file_path; else key.assign(buf, buf + len);

    Resource* r = createResourceFromFile(key);
    if (!r) return MeshPtr();

    auto it = m_resources.find(key);
    if (it == m_resources.end()) return MeshPtr();

    return std::static_pointer_cast<Mesh>(it->second);
}
