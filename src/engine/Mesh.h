#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>

class Shader;  // forward declaration

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Color;
    // glm::vec3 Normal;     // добавим позже, когда будет освещение
    // glm::vec2 TexCoords;  // и текстуры
};

class Mesh
{
public:
    Mesh();
    explicit Mesh(const std::string& modelPath);  // позже для загрузки из файла
    ~Mesh();

    void Draw(const Shader& shader, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const;

    // Для теста — создаём куб вручную
    void SetupCube();

private:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;   // индексы для экономии вершин

    std::vector<Vertex>   vertices;
    std::vector<unsigned int> indices;

    Shader* m_shader = nullptr;  // указатель, чтобы не копировать объект

    void SetupMesh();
};