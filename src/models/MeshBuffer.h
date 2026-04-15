#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "../opengl/GLFunctions.h" // Используем ручную загрузку функций

namespace OGLE {

// Представляет данные одного меша (вершины, индексы) и буферы OpenGL
class MeshBuffer {
public:
    MeshBuffer();
    ~MeshBuffer();

    // Запрещаем копирование, чтобы избежать двойного удаления ресурсов GPU
    MeshBuffer(const MeshBuffer&) = delete;
    MeshBuffer& operator=(const MeshBuffer&) = delete;

    void Create(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    void Update(const std::vector<float>& vertices);
    void Draw() const; // Отрисовать меш

private:
    GLuint VAO = 0, VBO = 0, EBO = 0; // ID буферов OpenGL
    GLsizei m_indexCount = 0;
    GLsizeiptr m_vertexBufferSize = 0;
};

} // namespace OGLE