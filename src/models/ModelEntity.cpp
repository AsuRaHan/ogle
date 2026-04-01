#include "ModelEntity.h"
#include "../Logger.h"

namespace glm {
    void to_json(nlohmann::json& j, const vec3& v) {
        j = {v.x, v.y, v.z};
    }

    void from_json(const nlohmann::json& j, vec3& v) {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
    }
}

namespace OGLE {
    ModelEntity::ModelEntity(ModelType type, std::string filePath)
        : m_Type(type), m_Position(0.0f), m_Rotation(0.0f), m_Scale(1.0f), m_FilePath(filePath) {
        UpdateModelMatrix();
    }

    ModelEntity::~ModelEntity() {}

    void ModelEntity::Draw() {
        if (m_MeshBuffer) {
            // Here we would set the model matrix in the shader
            // For now, just bind and draw
            m_MeshBuffer->Bind();
            glDrawElements(GL_TRIANGLES, m_MeshBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0);
            m_MeshBuffer->Unbind();
        }
    }

    void ModelEntity::ConvertToStatic() {
        if (m_Type == ModelType::DYNAMIC) {
            BakeToGPU();
            m_Mesh.reset(); // Free OpenMesh data
            m_Type = ModelType::STATIC;
            LOG_INFO("Model converted to STATIC.");
        }
    }

    void ModelEntity::UpdateGeometry() {
        if (m_Type == ModelType::DYNAMIC && m_Mesh) {
            BakeToGPU();
            LOG_INFO("Model geometry updated.");
        } else {
            LOG_WARN("Cannot update geometry on a STATIC model or model with no mesh data.");
        }
    }

    void ModelEntity::SetMeshData(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        m_MeshBuffer = std::make_unique<MeshBuffer>();
        m_MeshBuffer->Create(vertices, indices);
        m_Type = ModelType::STATIC;
        m_FilePath.clear();
    }

    void ModelEntity::SetPosition(const glm::vec3& position) {
        m_Position = position;
        UpdateModelMatrix();
    }

    void ModelEntity::SetRotation(const glm::vec3& rotation) {
        m_Rotation = rotation;
        UpdateModelMatrix();
    }

    void ModelEntity::SetScale(const glm::vec3& scale) {
        m_Scale = scale;
        UpdateModelMatrix();
    }

    const glm::vec3& ModelEntity::GetPosition() const {
        return m_Position;
    }

    const glm::vec3& ModelEntity::GetRotation() const {
        return m_Rotation;
    }

    const glm::vec3& ModelEntity::GetScale() const {
        return m_Scale;
    }

    const glm::mat4& ModelEntity::GetModelMatrix() const {
        return m_ModelMatrix;
    }

    const std::string& ModelEntity::GetFilePath() const {
        return m_FilePath;
    }

    void ModelEntity::ToJson(nlohmann::json& j) const {
        j = nlohmann::json{
            {"filePath", m_FilePath},
            {"type", m_Type == ModelType::STATIC ? "STATIC" : "DYNAMIC"},
            {"position", {m_Position.x, m_Position.y, m_Position.z}},
            {"rotation", {m_Rotation.x, m_Rotation.y, m_Rotation.z}},
            {"scale", {m_Scale.x, m_Scale.y, m_Scale.z}}
        };
    }

    void ModelEntity::FromJson(const nlohmann::json& j) {
        m_FilePath = j.at("filePath").get<std::string>();
        std::string type = j.at("type").get<std::string>();
        m_Type = (type == "STATIC") ? ModelType::STATIC : ModelType::DYNAMIC;

        j.at("position").get_to(m_Position);
        j.at("rotation").get_to(m_Rotation);
        j.at("scale").get_to(m_Scale);

        LoadFromFile(m_FilePath);
        UpdateModelMatrix();
    }

    void ModelEntity::UpdateModelMatrix() {
        m_ModelMatrix = glm::mat4(1.0f);
        m_ModelMatrix = glm::translate(m_ModelMatrix, m_Position);
        m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(m_Rotation.x), glm::vec3(1, 0, 0));
        m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
        m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
        m_ModelMatrix = glm::scale(m_ModelMatrix, m_Scale);
    }
}

