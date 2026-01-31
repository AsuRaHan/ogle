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
        // Вершины куба с tangent/bitangent (упрощённо; для каждой грани рассчитай реальные, если нужно)
        std::vector<Vertex> vertices = {
            // Передняя грань (z=0.5)
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
            // ... (добавь остальные грани аналогично, с правильными normal/tangent/bitangent)
            // Для примера сократил; полный куб — 24 вершины (по 4 на грань) или 8 с индексами
        };

        std::vector<unsigned int> indices = {
            0,1,2,2,3,0,  // Передняя
            // ... (твои старые индексы, адаптируй)
        };

        CreateGeometry(vertices, indices);
        ComputeBoundingRadius(vertices);

        // Дефолтный материал для куба
        m_material = std::make_shared<BasicMaterial>();
        m_material->SetColor({ 1.0f, 0.5f, 0.0f, 1.0f });  // Пример оранжевый
    }

    void Mesh::Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
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
        // Добавь globals: shader->SetFloat("uTime", time); если шейдер использует

        m_material->Apply(shader);  // Set params, bind textures, apply RenderState

        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        shader->Unbind();
    }

} // namespace ogle