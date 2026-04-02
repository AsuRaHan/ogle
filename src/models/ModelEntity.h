#pragma once

#include "BaseModel.h"
#include "../render/Material.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace OGLE {
    enum class ModelType {
        STATIC,
        DYNAMIC
    };

    class ModelEntity : public BaseModel {
    public:
        ModelEntity(ModelType type = ModelType::DYNAMIC, std::string filePath = "");
        ~ModelEntity();

        void Draw();
        void BindMaterial(GLuint program) const;
        void ConvertToStatic();
        void UpdateGeometry();
        void SetMeshData(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
        void UpdateGpuData(); // Добавлено для обновления GPU буфера
        std::vector<float>& GetVertices(); // Добавлено для доступа к изменяемым вершинам
        const std::vector<float>& GetVertices() const; // Добавлено для чтения вершин
        const std::vector<unsigned int>& GetIndices() const; // Добавлено для чтения индексов
        bool SetDiffuseTexturePath(const std::string& texturePath);
        const std::string& GetDiffuseTexturePath() const;
        Material& GetMaterial();
        const Material& GetMaterial() const;
        
        ModelType GetType() const; // Добавлено для получения типа модели
        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& rotation);
        void SetScale(const glm::vec3& scale);
        const glm::vec3& GetPosition() const;
        const glm::vec3& GetRotation() const;
        const glm::vec3& GetScale() const;
        
        const glm::mat4& GetModelMatrix() const;
        const std::string& GetFilePath() const;

        void ToJson(nlohmann::json& j) const;
        void FromJson(const nlohmann::json& j);

    private:
        void UpdateModelMatrix();

        ModelType m_Type;
        glm::vec3 m_Position;
        glm::vec3 m_Rotation;
        glm::vec3 m_Scale;
        glm::mat4 m_ModelMatrix;
        std::string m_FilePath;
        Material m_material;
    };
}
