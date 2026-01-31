// src/scene/Mesh.h
#pragma once

#include "SceneComponent.h"
#include <glad/gl.h>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "render/material/Material.h"  // Для Material
#include "render/shader/ShaderProgram.h"
#include "log/Logger.h"

namespace ogle {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 tangent;   // Для normal mapping
        glm::vec3 bitangent; // Для normal mapping
        glm::vec3 color = { 1.0f, 1.0f, 1.0f };  // Default белый
    };

    class Mesh : public SceneComponent {
    public:
        Mesh();
        ~Mesh();

        bool Initialize();
        void CreateCube();  // Тестовый куб (для примера)
        void Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) override;
        float GetBoundingRadius() const { return m_boundingRadius; }

        std::shared_ptr<Material> GetMaterial() const { return m_material; }
        void SetMaterial(std::shared_ptr<Material> material) { m_material = material; }

        void CreateGeometry(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
        void ComputeBoundingRadius(const std::vector<Vertex>& vertices);
    private:
        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ebo = 0;
        size_t m_indexCount = 0;

        std::shared_ptr<Material> m_material;  // Материал для рендера
        float m_boundingRadius = 0.0f;


    };

} // namespace ogle