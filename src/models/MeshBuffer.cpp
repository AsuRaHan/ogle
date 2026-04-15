#include "MeshBuffer.h"
#include "../opengl/OpenGLUtils.h" // Для GL_CHECK
#include "../Logger.h"

namespace OGLE {

MeshBuffer::MeshBuffer()
    : VAO(0), VBO(0), EBO(0), m_indexCount(0)
{
}

MeshBuffer::~MeshBuffer() {
    if (VAO != 0) GL_CHECK(glDeleteVertexArrays(1, &VAO));
    if (VBO != 0) GL_CHECK(glDeleteBuffers(1, &VBO));
    if (EBO != 0) GL_CHECK(glDeleteBuffers(1, &EBO));
}

void MeshBuffer::Create(const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
{
    if (vertices.empty() || indices.empty()) {
        return;
    }

    m_indexCount = static_cast<GLsizei>(indices.size());
    m_vertexBufferSize = vertices.size() * sizeof(float);

    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glGenBuffers(1, &EBO));

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, m_vertexBufferSize, vertices.data(), GL_DYNAMIC_DRAW));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW));

    // Stride - это размер одной вершины в байтах. В BaseModel.cpp данные пакуются как:
    // pos (3 float) + normal (3 float) + texCoord (2 float) = 8 floats
    const GLsizei stride = 8 * sizeof(float);

    // Атрибут 0: Позиции вершин (vec3)
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0));

    // Атрибут 1: Нормали вершин (vec3)
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))));

    // Атрибут 2: Текстурные координаты вершин (UV) (vec2)
    GL_CHECK(glEnableVertexAttribArray(2));
    GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))));

    GL_CHECK(glBindVertexArray(0));
}

void MeshBuffer::Update(const std::vector<float>& vertices)
{
    if (VBO == 0) return;

    const GLsizeiptr newSize = vertices.size() * sizeof(float);
    if (newSize > m_vertexBufferSize) {
        LOG_ERROR("MeshBuffer::Update error: new vertex data is larger than the allocated buffer. Re-creation is needed.");
        // For simplicity, we don't re-allocate here. The user should create a new mesh.
        return;
    }

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, vertices.data()));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void MeshBuffer::Draw() const {
    if (VAO == 0 || m_indexCount == 0) return;
    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));
}

} // namespace OGLE