#pragma once

#include <string>
#include <memory>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include "MeshBuffer.h"

namespace OGLE {
    typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

    class BaseModel {
    public:
        BaseModel();
        ~BaseModel();

        bool LoadFromFile(const std::string& path);
        void BakeToGPU();

    protected:
        std::unique_ptr<MyMesh> m_Mesh;
        std::unique_ptr<MeshBuffer> m_MeshBuffer;
    };
}
