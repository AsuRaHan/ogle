// src/scene/Model.cpp
#include "Model.h"

namespace ogle {

Model::Model(const std::string& path, Scene* scene) {
    if (!scene) {
        Logger::Error("Model: Null scene pointer");
        return;
    }

    Assimp::Importer importer;
    const aiScene* aiScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
    if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode) {
        Logger::Error("Assimp load failed: " + std::string(importer.GetErrorString()));
        return;
    }

    m_directory = std::filesystem::path(path).parent_path();
    m_rootEntity = scene->CreateEntity(aiScene->mRootNode->mName.C_Str());
    ProcessNode(aiScene->mRootNode, aiScene, m_rootEntity, scene);
    Logger::Info("Model loaded: " + path + " (root entity: " + std::to_string(static_cast<uint32_t>(m_rootEntity)) + ")");
}

void Model::ProcessNode(aiNode* node, const aiScene* aiScene, entt::entity parentEntity, Scene* scene) {
    entt::entity current = scene->CreateEntity(node->mName.C_Str());
    scene->SetParent(current, parentEntity);

    // Трансформ из aiNode
    auto& tr = scene->registry.get<Transform>(current);
    aiMatrix4x4t<float> aiTr = node->mTransformation;
    glm::mat4 mat = glm::transpose(glm::make_mat4(&aiTr.a1));  // Assimp row-major -> glm column-major
    glm::vec3 skew;
    glm::vec4 persp;
    glm::decompose(mat, tr.scale, tr.rotation, tr.position, skew, persp);
    tr.MarkDirty();

    // Меши
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* aiMesh = aiScene->mMeshes[node->mMeshes[i]];
        ProcessMesh(aiMesh, aiScene, current, scene);
    }

    // Рекурсия для детей
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(node->mChildren[i], aiScene, current, scene);
    }
}

void Model::ProcessMesh(aiMesh* aiMesh, const aiScene* aiScene, entt::entity entity, Scene* scene) {
    // Создаём Mesh компонент
    auto& mesh = scene->registry.emplace<Mesh>(entity);
    mesh.Initialize();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int j = 0; j < aiMesh->mNumVertices; ++j) {
        Vertex vertex;
        vertex.position = { aiMesh->mVertices[j].x, aiMesh->mVertices[j].y, aiMesh->mVertices[j].z };
        if (aiMesh->HasNormals()) {
            vertex.normal = { aiMesh->mNormals[j].x, aiMesh->mNormals[j].y, aiMesh->mNormals[j].z };
        }
        if (aiMesh->mTextureCoords[0]) {
            vertex.uv = { aiMesh->mTextureCoords[0][j].x, aiMesh->mTextureCoords[0][j].y };
        }
        if (aiMesh->HasTangentsAndBitangents()) {
            vertex.tangent = { aiMesh->mTangents[j].x, aiMesh->mTangents[j].y, aiMesh->mTangents[j].z };
            vertex.bitangent = { aiMesh->mBitangents[j].x, aiMesh->mBitangents[j].y, aiMesh->mBitangents[j].z };
        }
        if (aiMesh->HasVertexColors(0)) {
            vertex.color = { aiMesh->mColors[0][j].r, aiMesh->mColors[0][j].g, aiMesh->mColors[0][j].b };
        }
        vertices.push_back(vertex);
    }

    for (unsigned int j = 0; j < aiMesh->mNumFaces; ++j) {
        aiFace face = aiMesh->mFaces[j];
        for (unsigned int k = 0; k < face.mNumIndices; ++k) {
            indices.push_back(face.mIndices[k]);
        }
    }

    mesh.CreateGeometry(vertices, indices);  // Приватный метод в Mesh
    mesh.ComputeBoundingRadius(vertices);    // Приватный

    // Материал
    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = aiScene->mMaterials[aiMesh->mMaterialIndex];
        ProcessMaterial(aiMat, entity, scene);
    }
}

void Model::ProcessMaterial(aiMaterial* aiMat, entt::entity entity, Scene* scene) {
    auto& mesh = scene->registry.get<Mesh>(entity);

    // Определяем тип материала
    float metallicFactor = 0.0f;
    aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
    std::shared_ptr<Material> material;
    if (metallicFactor > 0.0f) {
        material = std::make_shared<PBRMaterial>();
    } else {
        material = std::make_shared<BasicMaterial>();
    }

    // Цвет
    aiColor4D color;
    if (aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
        glm::vec4 albedo(color.r, color.g, color.b, color.a);
        if (auto* basic = dynamic_cast<BasicMaterial*>(material.get())) {
            basic->SetColor(albedo);
        } else if (auto* pbr = dynamic_cast<PBRMaterial*>(material.get())) {
            pbr->SetAlbedo(albedo);
        }
    }

    // Текстуры
    auto& texCtrl = TextureController::Get();
    auto loadTex = [&](aiTextureType type, const std::string& paramName) {
        if (aiMat->GetTextureCount(type) > 0) {
            aiString texPath;
            if (aiMat->GetTexture(type, 0, &texPath) == AI_SUCCESS) {
                std::string fullPath = (m_directory / texPath.data).string();
                Texture* tex = texCtrl.LoadTexture2D(fullPath);
                if (tex) {
                    material->SetTexture(paramName, tex);
                }
            }
        }
    };

    loadTex(aiTextureType_DIFFUSE, "albedoMap");
    loadTex(aiTextureType_HEIGHT, "normalMap");
    loadTex(aiTextureType_SPECULAR, "specularMap");
    loadTex(aiTextureType_SHININESS, "roughnessMap");
    loadTex(aiTextureType_DISPLACEMENT, "heightMap");
    // Добавь больше по нужде

    mesh.SetMaterial(material);
}

} // namespace ogle