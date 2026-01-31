// src/scene/Model.h
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <string>
#include <filesystem>
#include "Scene.h"  // Для Scene и entt::entity
#include "Mesh.h"   // Для Mesh компонента
#include "render/TextureController.h"
#include "log/Logger.h"

namespace ogle {

class Model {
public:
    Model(const std::string& path, Scene* scene);
    ~Model() = default;

    entt::entity GetRootEntity() const { return m_rootEntity; }

private:
    entt::entity m_rootEntity = entt::null;
    std::filesystem::path m_directory;

    void ProcessNode(aiNode* node, const aiScene* aiScene, entt::entity parentEntity, Scene* scene);
    void ProcessMesh(aiMesh* aiMesh, const aiScene* aiScene, entt::entity entity, Scene* scene);
    void ProcessMaterial(aiMaterial* aiMat, entt::entity entity, Scene* scene);  // Создаёт Material для Mesh
};

} // namespace ogle