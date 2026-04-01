#include "DomoScene.h"
#include "../Logger.h"

#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM 0x8B8D
#endif
// Подключаем для glm::translate, rotate, scale
#include <glm/gtc/matrix_transform.hpp>

DomoScene::DomoScene()
    : m_vao(0)
    , m_vbo(0)
    , m_vertexCount(0)
{}

DomoScene::~DomoScene() {
    destroySceneResources();
}

bool DomoScene::Initialize() {
    LOG_INFO("DomoScene: инициализация сцены");
    return createSceneResources();
}

void DomoScene::Render(float deltaTime) {
    // Compute model matrix from stored size, position and rotation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);          // <‑‑ исправлено
    model = glm::rotate(model, m_rotationAngle, m_rotationAxis); // <‑‑ исправлено
    model = glm::scale(model, m_size);                  // <‑‑ исправлено
    glm::mat4 mvp = model;  // без view/proj
    GLint program = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program != 0) {
        GLint loc = glGetUniformLocation(static_cast<GLuint>(program), "uMVP");
        if (loc >= 0) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, &mvp[0][0]);
        }
    }

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);
}

bool DomoScene::createSceneResources() {
    // 3D дом:
    // квадраты стен + четырёхугольная крыша сверху (пирамида)
    float vertices[] = {
        // передняя стена (z = 0.5)
        -0.5f, -0.5f,  0.5f,  0.6f, 0.4f, 0.2f,
         0.5f, -0.5f,  0.5f,  0.6f, 0.4f, 0.2f,
         0.5f,  0.5f,  0.5f,  0.7f, 0.5f, 0.3f,
        -0.5f, -0.5f,  0.5f,  0.6f, 0.4f, 0.2f,
         0.5f,  0.5f,  0.5f,  0.7f, 0.5f, 0.3f,
        -0.5f,  0.5f,  0.5f,  0.7f, 0.5f, 0.3f,

        // задняя стена (z = -0.5)
        -0.5f, -0.5f, -0.5f,  0.5f, 0.35f,0.18f,
         0.5f, -0.5f, -0.5f,  0.5f, 0.35f,0.18f,
         0.5f,  0.5f, -0.5f,  0.65f,0.45f,0.27f,
        -0.5f, -0.5f, -0.5f,  0.5f, 0.35f,0.18f,
         0.5f,  0.5f, -0.5f,  0.65f,0.45f,0.27f,
        -0.5f,  0.5f, -0.5f,  0.65f,0.45f,0.27f,

        // левая стена (x = -0.5)
        -0.5f, -0.5f,  0.5f,  0.55f,0.4f,0.24f,
        -0.5f, -0.5f, -0.5f,  0.55f,0.4f,0.24f,
        -0.5f,  0.5f, -0.5f,  0.7f,0.5f,0.3f,
        -0.5f, -0.5f,  0.5f,  0.55f,0.4f,0.24f,
        -0.5f,  0.5f, -0.5f,  0.7f,0.5f,0.3f,
        -0.5f,  0.5f,  0.5f,  0.7f,0.5f,0.3f,

        // правая стена (x = 0.5)
         0.5f, -0.5f,  0.5f,  0.5f,0.35f,0.22f,
         0.5f, -0.5f, -0.5f,  0.5f,0.35f,0.22f,
         0.5f,  0.5f, -0.5f,  0.65f,0.45f,0.28f,
         0.5f, -0.5f,  0.5f,  0.5f,0.35f,0.22f,
         0.5f,  0.5f, -0.5f,  0.65f,0.45f,0.28f,
         0.5f,  0.5f,  0.5f,  0.65f,0.45f,0.28f,

        // крыша: 4 треугольника, вершина (0,0.8,0)
        -0.6f,  0.5f,  0.6f,  0.9f,0.2f,0.1f,
         0.6f,  0.5f,  0.6f,  0.9f,0.2f,0.1f,
         0.0f,  0.9f,  0.0f,  0.95f,0.15f,0.1f,

         0.6f,  0.5f,  0.6f,  0.9f,0.2f,0.1f,
         0.6f,  0.5f, -0.6f,  0.9f,0.2f,0.1f,
         0.0f,  0.9f,  0.0f,  0.95f,0.15f,0.1f,

         0.6f,  0.5f, -0.6f,  0.9f,0.2f,0.1f,
        -0.6f,  0.5f, -0.6f,  0.9f,0.2f,0.1f,
         0.0f,  0.9f,  0.0f,  0.95f,0.15f,0.1f,

        -0.6f,  0.5f, -0.6f,  0.9f,0.2f,0.1f,
        -0.6f,  0.5f,  0.6f,  0.9f,0.2f,0.1f,
         0.0f,  0.9f,  0.0f,  0.95f,0.15f,0.1f
    };

    m_vertexCount = 6*4 + 4*3; // 24 + 12 = 36

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOG_ERROR("DomoScene createSceneResources OpenGL error: " + std::to_string(error));
        return false;
    }

    return true;
}

// Setters for transformation properties
void DomoScene::setSize(const glm::vec3& size) {
    m_size = size;
}

void DomoScene::setPosition(const glm::vec3& position) {
    m_position = position;
}

void DomoScene::setRotation(float angle, const glm::vec3& axis) {
    m_rotationAngle = angle;
    m_rotationAxis = axis;
}

void DomoScene::destroySceneResources() {
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}
