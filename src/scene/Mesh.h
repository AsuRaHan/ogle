// src/scene/Mesh.h
#pragma once

#include "SceneComponent.h"
#include <glad/gl.h>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "../render/shader/ShaderProgram.h"  // Твой шейдер-контроллер
#include "../log/Logger.h"

namespace ogle {

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;      // Для теста, можно расширить (normal, uv и т.д.)
    // Добавь позже: glm::vec3 normal; glm::vec2 uv;
};

class Mesh : public SceneComponent {
public:
    Mesh();
    ~Mesh();

    bool Initialize();  // Создаёт VAO/VBO/EBO (пустой)
    void CreateCube();  // Тестовый куб, вызывай вручную когда нужно
    void LoadFromFile(const std::string& path);  // TODO: Загрузка из OBJ/FBX позже

    void Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) override;

    float GetBoundingRadius() const { return m_boundingRadius; }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    size_t m_indexCount = 0;

    std::shared_ptr<ShaderProgram> m_shaderProgram;
    float m_boundingRadius = 1.0f;  // Для culling, обновляется при загрузке

    void CreateGeometry(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

} // namespace ogle