#pragma once

#include "../opengl/GLFunctions.h"
#include <vector>

namespace OGLE {
    class MeshBuffer {
    public:
        MeshBuffer();
        ~MeshBuffer();

        void Create(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
        void Bind() const;
        void Unbind() const;

        GLuint GetVAO() const { return m_VAO; }
        GLuint GetVBO() const { return m_VBO; }
        GLuint GetEBO() const { return m_EBO; }
        size_t GetIndexCount() const { return m_IndexCount; }

    private:
        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        GLuint m_EBO = 0;
        size_t m_IndexCount = 0;
    };
}
