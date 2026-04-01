#pragma once

#include "BaseModel.h"
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
        void ConvertToStatic();
        void UpdateGeometry();
        void SetMeshData(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
        
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
    };
}
