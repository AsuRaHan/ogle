// src/scene/Mesh.cpp
#include "Mesh.h"

namespace ogle {

    Mesh::Mesh() {}

    Mesh::~Mesh() {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_ebo) glDeleteBuffers(1, &m_ebo);
    }

    bool Mesh::Initialize() {
        // Инициализация: Создаём дефолтный материал, если не задан
        if (!m_material) {
            m_material = std::make_shared<BasicMaterial>();
        }
        return true;
    }

    void Mesh::CreateGeometry(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
        if (vertices.empty() || indices.empty()) {
            Logger::Error("Empty geometry in Mesh");
            return;
        }

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Атрибуты
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
        glEnableVertexAttribArray(5);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        m_indexCount = indices.size();
    }

    void Mesh::ComputeBoundingRadius(const std::vector<Vertex>& vertices) {
        if (vertices.empty()) return;

        glm::vec3 center(0.0f);
        for (const auto& v : vertices) {
            center += v.position;
        }
        center /= static_cast<float>(vertices.size());

        m_boundingRadius = 0.0f;
        for (const auto& v : vertices) {
            m_boundingRadius = std::max(m_boundingRadius, glm::length(v.position - center));
        }
    }

    void Mesh::CreateCube() {
        // Куб [-0.5, 0.5]^3: 24 вершины (4 на грань), нормали/UV/tangent/bitangent для каждой грани
        const glm::vec3 white(1.0f, 1.0f, 1.0f);
        std::vector<Vertex> vertices = {
            // Передняя грань (Z+), normal (0,0,1), tangent (1,0,0), bitangent (0,1,0)
            {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, white},
            // Задняя грань (Z-), normal (0,0,-1), tangent (-1,0,0), bitangent (0,1,0)
            {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, white},
            // Правая грань (X+), normal (1,0,0), tangent (0,0,-1), bitangent (0,1,0)
            {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, white},
            // Левая грань (X-), normal (-1,0,0), tangent (0,0,1), bitangent (0,1,0)
            {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, white},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, white},
            // Верхняя грань (Y+), normal (0,1,0), tangent (1,0,0), bitangent (0,0,-1)
            {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, white},
            {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, white},
            {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, white},
            {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f, -1.0f}, white},
            // Нижняя грань (Y-), normal (0,-1,0), tangent (1,0,0), bitangent (0,0,1)
            {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f,  1.0f}, white},
            {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f,  1.0f}, white},
            {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f,  1.0f}, white},
            {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f,  1.0f}, white},
        };

        std::vector<unsigned int> indices = {
            0,  1,  2,  2,  3,  0,   // передняя (Z+)
            4,  5,  6,  6,  7,  4,   // задняя (Z-)
            8,  9, 10, 10, 11,  8,   // правая (X+)
            12, 13, 14, 14, 15, 12,  // левая (X-)
            16, 17, 18, 18, 19, 16,  // верх (Y+)
            20, 21, 22, 22, 23, 20,  // низ (Y-)
        };

        CreateGeometry(vertices, indices);
        ComputeBoundingRadius(vertices);

        auto& shaderCtrl = ShaderController::Get();
        m_material = std::make_shared<BasicMaterial>();
        m_material->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
        m_material->SetColor({ 1.0f, 0.5f, 0.0f, 1.0f });
    }

    void Mesh::Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const LightContext* lights) {
        if (!m_material) return;

        auto* shader = m_material->GetShader();
        if (!shader) {
            Logger::Warning("No shader in material for Mesh");
            return;
        }

        shader->Bind();
        shader->SetMat4("uModel", model);
        shader->SetMat4("uView", view);
        shader->SetMat4("uProjection", projection);

        m_material->Apply(shader);  // Set params, bind textures, apply RenderState

        LightContext::ApplyToShader(lights, shader, m_material->GetUseLighting());

        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        shader->Unbind();
    }

} // namespace ogle