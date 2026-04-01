#pragma once
#ifndef DOMOSCENE_H_
#define DOMOSCENE_H_

#include "GLFunctions.h"
#include <glm/glm.hpp>

class DomoScene {
public:
    DomoScene();
    ~DomoScene();

    bool Initialize();
    // Render теперь принимает только deltaTime. Трансформация (размер, позиция, поворот) задается через сеттеры.
    void Render(float deltaTime);
    // Установить размер объекта (масштаб)
    void setSize(const glm::vec3& size);
    // Установить позицию объекта
    void setPosition(const glm::vec3& position);
    // Установить поворот объекта (угол в радианах, ось вращения)
    void setRotation(float angle, const glm::vec3& axis);

private:
    bool createSceneResources();
    void destroySceneResources();

    GLuint m_vao;
    GLuint m_vbo;
    int m_vertexCount;
    // Transformation state
    glm::vec3 m_position{0.0f, 0.0f, 0.0f};
    glm::vec3 m_size{1.0f, 1.0f, 1.0f};
    float m_rotationAngle{0.0f};
    glm::vec3 m_rotationAxis{0.0f, 1.0f, 0.0f};
};

#endif // DOMOSCENE_H_
