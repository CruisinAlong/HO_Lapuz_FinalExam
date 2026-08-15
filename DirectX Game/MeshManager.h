#pragma once
#include "ResourceManager.h"
#include "Prerequisites.h"

class MeshManager : public ResourceManager
{
public:
    MeshManager();
    ~MeshManager();

    MeshPtr createMeshFromFile(const std::wstring& file_path);
    MeshPtr createCubeMesh();
    MeshPtr createPlaneMesh();

protected:
    Resource* createResourceFromFileConcrete(const std::wstring& absolute_path) override;
};
