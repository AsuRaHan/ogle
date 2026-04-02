#include "MeshBuffer.h"

namespace OGLE {
    MeshBuffer::MeshBuffer() {}

    MeshBuffer::~MeshBuffer() {
        if (m_VAO != 0) {
            glDeleteVertexArrays(1, &m_VAO);
        }
        if (m_VBO != 0) {
            glDeleteBuffers(1, &m_VBO);
        }
        if (m_EBO != 0) {
            glDeleteBuffers(1, &m_EBO);
        }
    }

    void MeshBuffer::Create(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        m_IndexCount = indices.size();

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW); // Изменено на GL_DYNAMIC_DRAW

        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void MeshBuffer::Update(const std::vector<float>& vertices)
    {
        if (m_VBO == 0) {
            return;
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void MeshBuffer::Bind() const {
        glBindVertexArray(m_VAO);
    }

    void MeshBuffer::Unbind() const {
        glBindVertexArray(0);
    }
}
