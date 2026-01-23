#include "Mesh.h"
#include "Shader.h"  // теперь подключаем

Mesh::Mesh()
{
    // По умолчанию создаём куб
    SetupCube();
    SetupMesh();
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Mesh::SetupCube()
{
    // 8 вершин куба (позиции + цвета)
    vertices = {
        // Передняя грань
        {{ -0.5f, -0.5f,  0.5f }, {1.0f, 0.0f, 0.0f}},  // 0
        {{  0.5f, -0.5f,  0.5f }, {0.0f, 1.0f, 0.0f}},  // 1
        {{  0.5f,  0.5f,  0.5f }, {0.0f, 0.0f, 1.0f}},  // 2
        {{ -0.5f,  0.5f,  0.5f }, {1.0f, 1.0f, 0.0f}},  // 3
        // Задняя грань
        {{ -0.5f, -0.5f, -0.5f }, {1.0f, 0.5f, 0.0f}},  // 4
        {{  0.5f, -0.5f, -0.5f }, {0.5f, 1.0f, 0.0f}},  // 5
        {{  0.5f,  0.5f, -0.5f }, {0.0f, 0.5f, 1.0f}},  // 6
        {{ -0.5f,  0.5f, -0.5f }, {1.0f, 0.0f, 0.5f}}   // 7
    };

    // 36 индексов (12 треугольников)
indices = {
    // Передняя
    0, 2, 3,  0, 1, 2,
    // Задняя
    4, 6, 5,  4, 7, 6,
    // Низ
    0, 5, 1,  0, 4, 5,
    // Верх
    2, 6, 7,  2, 7, 3,
    // Левая
    0, 7, 4,  0, 3, 7,
    // Правая
    1, 6, 2,  1, 5, 6
};
}

void Mesh::SetupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Позиция
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);

    // Цвет
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Mesh::Draw(const Shader& shader, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const
{
    shader.Use();
    shader.SetMat4("model", model);
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}