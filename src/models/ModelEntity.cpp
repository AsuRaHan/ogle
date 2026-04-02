#include "ModelEntity.h"
#include "../Logger.h"

namespace glm {
    void to_json(nlohmann::json& j, const vec2& v) {
        j = {v.x, v.y};
    }

    void from_json(const nlohmann::json& j, vec2& v) {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
    }

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
            m_MeshBuffer->Bind();
            glDrawElements(GL_TRIANGLES, m_MeshBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0);
            m_MeshBuffer->Unbind();
        }
    }

    void ModelEntity::BindMaterial(GLuint program) const
    {
        m_material.Bind(program);
    }

    void ModelEntity::ConvertToStatic() {
        if (m_Type == ModelType::DYNAMIC) {
            BakeToGPU();
            m_vertices.clear();
            m_indices.clear();
            m_Type = ModelType::STATIC;
            LOG_INFO("Model converted to STATIC.");
        }
    }

    void ModelEntity::UpdateGeometry() {
        if (m_Type == ModelType::DYNAMIC && !m_vertices.empty() && !m_indices.empty()) {
            BakeToGPU();
            LOG_INFO("Model geometry updated.");
        } else {
            LOG_WARN("Cannot update geometry on a STATIC model or model with no mesh data.");
        }
    }

    void ModelEntity::SetMeshData(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        SetMeshGeometry(vertices, indices);
        BakeToGPU();
        m_Type = ModelType::STATIC;
        m_FilePath.clear();
    }

    void ModelEntity::UpdateGpuData()
    {
        if (m_MeshBuffer) {
            m_MeshBuffer->Update(m_vertices);
        }
    }

    std::vector<float>& ModelEntity::GetVertices() {
        return m_vertices;
    }

    const std::vector<float>& ModelEntity::GetVertices() const {
        return m_vertices;
    }

    const std::vector<unsigned int>& ModelEntity::GetIndices() const
    {
        return m_indices;
    }

    bool ModelEntity::SetDiffuseTexturePath(const std::string& texturePath)
    {
        return m_material.SetDiffuseTexturePath(texturePath);
    }

    const std::string& ModelEntity::GetDiffuseTexturePath() const
    {
        return m_material.GetDiffuseTexturePath();
    }

    Material& ModelEntity::GetMaterial()
    {
        return m_material;
    }

    const Material& ModelEntity::GetMaterial() const
    {
        return m_material;
    }

    ModelType ModelEntity::GetType() const
    {
        return m_Type;
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
        const Material& material = m_material;
        j = nlohmann::json{
            {"filePath", m_FilePath},
            {"type", m_Type == ModelType::STATIC ? "STATIC" : "DYNAMIC"},
            {"position", {m_Position.x, m_Position.y, m_Position.z}},
            {"rotation", {m_Rotation.x, m_Rotation.y, m_Rotation.z}},
            {"scale", {m_Scale.x, m_Scale.y, m_Scale.z}},
            {"material", {
                {"baseColor", {material.GetBaseColor().x, material.GetBaseColor().y, material.GetBaseColor().z}},
                {"emissiveColor", {material.GetEmissiveColor().x, material.GetEmissiveColor().y, material.GetEmissiveColor().z}},
                {"uvTiling", {material.GetUvTiling().x, material.GetUvTiling().y}},
                {"uvOffset", {material.GetUvOffset().x, material.GetUvOffset().y}},
                {"roughness", material.GetRoughness()},
                {"metallic", material.GetMetallic()},
                {"alphaCutoff", material.GetAlphaCutoff()},
                {"diffuseTexturePath", material.GetDiffuseTexturePath()},
                {"emissiveTexturePath", material.GetEmissiveTexturePath()}
            }}
        };

        if (m_FilePath.empty() && !m_vertices.empty() && !m_indices.empty()) {
            j["geometry"] = {
                {"vertices", m_vertices},
                {"indices", m_indices}
            };
        }
    }

    void ModelEntity::FromJson(const nlohmann::json& j) {
        m_FilePath = j.at("filePath").get<std::string>();
        std::string type = j.at("type").get<std::string>();
        m_Type = (type == "STATIC") ? ModelType::STATIC : ModelType::DYNAMIC;

        j.at("position").get_to(m_Position);
        j.at("rotation").get_to(m_Rotation);
        j.at("scale").get_to(m_Scale);

        if (!m_FilePath.empty()) {
            LoadFromFile(m_FilePath);
            BakeToGPU();
        } else if (j.contains("geometry")) {
            const auto& geometryJson = j.at("geometry");
            SetMeshGeometry(
                geometryJson.at("vertices").get<std::vector<float>>(),
                geometryJson.at("indices").get<std::vector<unsigned int>>());
            BakeToGPU();
        }

        if (j.contains("material")) {
            const auto& materialJson = j.at("material");
            if (materialJson.contains("baseColor")) {
                const auto& baseColorJson = materialJson.at("baseColor");
                m_material.SetBaseColor(glm::vec3(baseColorJson[0], baseColorJson[1], baseColorJson[2]));
            }
            if (materialJson.contains("emissiveColor")) {
                const auto& emissiveColorJson = materialJson.at("emissiveColor");
                m_material.SetEmissiveColor(glm::vec3(emissiveColorJson[0], emissiveColorJson[1], emissiveColorJson[2]));
            }
            if (materialJson.contains("uvTiling")) {
                const auto& uvTilingJson = materialJson.at("uvTiling");
                m_material.SetUvTiling(glm::vec2(uvTilingJson[0], uvTilingJson[1]));
            }
            if (materialJson.contains("uvOffset")) {
                const auto& uvOffsetJson = materialJson.at("uvOffset");
                m_material.SetUvOffset(glm::vec2(uvOffsetJson[0], uvOffsetJson[1]));
            }
            if (materialJson.contains("roughness")) {
                m_material.SetRoughness(materialJson.at("roughness").get<float>());
            }
            if (materialJson.contains("metallic")) {
                m_material.SetMetallic(materialJson.at("metallic").get<float>());
            }
            if (materialJson.contains("alphaCutoff")) {
                m_material.SetAlphaCutoff(materialJson.at("alphaCutoff").get<float>());
            }

            if (materialJson.contains("diffuseTexturePath")) {
                m_material.SetDiffuseTexturePath(materialJson.at("diffuseTexturePath").get<std::string>());
            }
            if (materialJson.contains("emissiveTexturePath")) {
                m_material.SetEmissiveTexturePath(materialJson.at("emissiveTexturePath").get<std::string>());
            }
        } else if (!GetLoadedDiffuseTexturePath().empty()) {
            m_material.SetDiffuseTexturePath(GetLoadedDiffuseTexturePath());
        }

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
