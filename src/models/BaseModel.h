#pragma once

#include <string>
#include <memory>
#include <vector>
#include "MeshBuffer.h"

namespace OGLE {
    class BaseModel {
    public:
        BaseModel();
        ~BaseModel();

        bool LoadFromFile(const std::string& path);
        void BakeToGPU();
        const std::string& GetLoadedDiffuseTexturePath() const;

    protected:
        void SetMeshGeometry(std::vector<float> vertices, std::vector<unsigned int> indices);

        std::vector<float> m_vertices;
        std::vector<unsigned int> m_indices;
        std::unique_ptr<MeshBuffer> m_MeshBuffer;
        std::string m_loadedDiffuseTexturePath;
    };
}
