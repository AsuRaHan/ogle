// src/scene/Mesh.cpp
#include "Mesh.h"
#include "render/ShaderController.h"

namespace ogle {

Mesh::Mesh() {}

Mesh::~Mesh() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

bool Mesh::Initialize() {
    auto& shaderCtrl = ShaderController::Get();
    m_shaderProgram = shaderCtrl.CreateProgram("MeshDefault",
        // Вертекс шейдер
        R"(#version 460 core
           layout(location = 0) in vec3 aPos;
           layout(location = 1) in vec3 aColor;
           uniform mat4 uModel;
           uniform mat4 uView;
           uniform mat4 uProjection;
           out vec3 vColor;
           void main() {
               vColor = aColor;
               gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
           })",
        // Фрагмент
        R"(#version 460 core
           in vec3 vColor;
           out vec4 FragColor;
           void main() { FragColor = vec4(vColor, 1.0); })"
    );

    if (!m_shaderProgram) {
        Logger::Error("Не удалось создать шейдер для Mesh");
        return false;
    }
    return true;
}

void Mesh::CreateCube() {
    // Полные вершины куба (как в твоём оригинальном коде)
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}}
    };

    std::vector<unsigned int> indices = {
        0,1,2,2,3,0, 4,5,6,6,7,4,
        8,9,10,10,11,8, 12,13,14,14,15,12,
        16,17,18,18,19,16, 20,21,22,22,23,20
    };

    CreateGeometry(vertices, indices);
    m_boundingRadius = 0.866f;  // sqrt(3)/2 для единичного куба
}

void Mesh::CreateGeometry(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_indexCount = indices.size();
    
    // Обновляем bounding (простой расчёт радиуса)
    m_boundingRadius = 0.0f;
    for (const auto& v : vertices) {
        m_boundingRadius = std::max(m_boundingRadius, glm::length(v.position));
    }
}

void Mesh::Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    if (!m_shaderProgram || m_vao == 0) return;

    m_shaderProgram->Bind();
    m_shaderProgram->SetMat4("uModel", model);
    m_shaderProgram->SetMat4("uView", view);
    m_shaderProgram->SetMat4("uProjection", projection);
    // Добавь time если анимация: m_shaderProgram->SetFloat("uTime", time);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    m_shaderProgram->Unbind();
}

void Mesh::LoadFromFile(const std::string& path) {
    // TODO: Реализация загрузки из OBJ/FBX (используй assimp или ручной парсер)
    Logger::Info("Загрузка меша из " + path + " (не реализовано)");
}

} // namespace ogle