#pragma once

#include <string>
#include <memory>
#include <vector>
#include "MeshBuffer.h"
#include "../world/WorldComponents.h"

namespace OGLE {
    class BaseModel {
    public:
        BaseModel();
        ~BaseModel();

        bool LoadFromFile(const std::string& path);
        bool LoadCustomFile(const std::string& path);
        bool SaveToCustomFile(const std::string& path) const;
        void BakeToGPU();
        const std::string& GetLoadedDiffuseTexturePath() const;
        const std::vector<AnimationClip>& GetAnimationClips() const;
        void SetAnimationClips(const std::vector<AnimationClip>& clips);
        int GetBoneCount() const;

    protected:
        void SetMeshGeometry(std::vector<float> vertices, std::vector<unsigned int> indices);

        std::vector<AnimationClip> m_animationClips;
        std::vector<float> m_vertices;
        std::vector<unsigned int> m_indices;
        std::unique_ptr<MeshBuffer> m_MeshBuffer;
        std::string m_loadedDiffuseTexturePath;
        int m_boneCount = 0;
    };
}
