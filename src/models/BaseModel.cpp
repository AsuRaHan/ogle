#include "BaseModel.h"
#include "../Logger.h"
#include "../core/FileSystem.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>

namespace OGLE {
    namespace
    {
        std::string ResolveDiffuseTexturePath(
            const aiScene* scene,
            const aiMesh* mesh,
            const std::filesystem::path& modelPath)
        {
            if (!scene || !mesh || mesh->mMaterialIndex >= scene->mNumMaterials) {
                return {};
            }

            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            if (!material || material->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
                return {};
            }

            aiString texturePath;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) != aiReturn_SUCCESS) {
                return {};
            }

            const std::string rawTexturePath = texturePath.C_Str();
            if (rawTexturePath.empty()) {
                return {};
            }

            if (rawTexturePath[0] == '*') {
                LOG_WARN("Embedded diffuse textures are not supported yet for: " + modelPath.string());
                return {};
            }

            const std::filesystem::path candidatePath = modelPath.parent_path() / rawTexturePath;
            return FileSystem::ResolvePath(candidatePath).string();
        }
    }

    BaseModel::BaseModel() = default;

    BaseModel::~BaseModel() {}

    bool BaseModel::LoadFromFile(const std::string& path) {
        const std::filesystem::path resolvedPath = FileSystem::ResolvePath(path);
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            resolvedPath.string(),
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality |
            aiProcess_FlipUVs |
            aiProcess_SortByPType);

        if (!scene || !scene->HasMeshes()) {
            LOG_ERROR("Error loading mesh from file: " + path);
            return false;
        }

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::string diffuseTexturePath;

        for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
            const aiMesh* mesh = scene->mMeshes[meshIndex];
            if (!mesh) {
                continue;
            }

            if (diffuseTexturePath.empty()) {
                diffuseTexturePath = ResolveDiffuseTexturePath(scene, mesh, resolvedPath);
            }

            const unsigned int vertexOffset = static_cast<unsigned int>(vertices.size() / 8);
            for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
                const aiVector3D& position = mesh->mVertices[vertexIndex];
                const aiVector3D normal = mesh->HasNormals()
                    ? mesh->mNormals[vertexIndex]
                    : aiVector3D(0.0f, 1.0f, 0.0f);
                const aiVector3D texCoord = mesh->HasTextureCoords(0)
                    ? mesh->mTextureCoords[0][vertexIndex]
                    : aiVector3D(0.0f, 0.0f, 0.0f);

                vertices.push_back(position.x);
                vertices.push_back(position.y);
                vertices.push_back(position.z);
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);
                vertices.push_back(texCoord.x);
                vertices.push_back(texCoord.y);
            }

            for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
                const aiFace& face = mesh->mFaces[faceIndex];
                if (face.mNumIndices != 3) {
                    continue;
                }

                indices.push_back(vertexOffset + face.mIndices[0]);
                indices.push_back(vertexOffset + face.mIndices[1]);
                indices.push_back(vertexOffset + face.mIndices[2]);
            }
        }

        if (vertices.empty() || indices.empty()) {
            LOG_ERROR("Mesh file contains no renderable geometry: " + resolvedPath.string());
            return false;
        }

        SetMeshGeometry(std::move(vertices), std::move(indices));
        m_loadedDiffuseTexturePath = std::move(diffuseTexturePath);
        return true;
    }

    void BaseModel::BakeToGPU() {
        if (m_vertices.empty() || m_indices.empty()) {
            LOG_ERROR("Cannot bake to GPU, mesh data is missing.");
            return;
        }

        m_MeshBuffer = std::make_unique<MeshBuffer>();
        m_MeshBuffer->Create(m_vertices, m_indices);
    }

    const std::string& BaseModel::GetLoadedDiffuseTexturePath() const
    {
        return m_loadedDiffuseTexturePath;
    }

    void BaseModel::SetMeshGeometry(std::vector<float> vertices, std::vector<unsigned int> indices)
    {
        m_vertices = std::move(vertices);
        m_indices = std::move(indices);
    }
}
