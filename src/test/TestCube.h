// src/test/TestCube.h
#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "render/shader/ShaderProgram.h"

namespace ogle {

class TestCube {
public:
    enum class ShaderMode {
        Colorful,      // наш кастомный разноцветный
        BasicColor,    // builtin BasicColor
        BasicTexture,  // builtin BasicTexture (без текстуры)
        Unlit,         // builtin Unlit
        Wireframe      // builtin Wireframe
    };

    void SetShaderMode(ShaderMode mode);

    TestCube();
    ~TestCube();
    
    bool Initialize();
    void Shutdown();
    
    // Обновление (для обработки ввода и логики)
    void Update(float deltaTime);
    
    // Рендер с камерой
    void Render(float time, const glm::mat4& view, const glm::mat4& projection);
    // Fallback рендер (без камеры)
    void Render(float time);
    
    // Тестирование builtin шейдеров
    bool TestBuiltinShaders();

    // Настройка системы ввода
    void SetupInputActions();
    
    // Управление камерой
    void SetCameraControlEnabled(bool enabled) { m_cameraControlEnabled = enabled; }
    bool IsCameraControlEnabled() const { return m_cameraControlEnabled; }
    
private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    
    std::shared_ptr<ShaderProgram> m_shaderProgram;
    
    // Для управления камерой
    bool m_cameraControlEnabled = false;
    glm::vec2 m_lastMousePos = {0.0f, 0.0f};
    bool m_rightMouseDown = false;
    
    // Для тестирования осей
    float m_testRotationSpeed = 1.0f;
    float m_testZoom = 1.0f;
    
    // Вспомогательные методы
    void CreateGeometry();
    void ProcessCameraInput(float deltaTime);
    void CycleShader();
    
    // Статистика для отладки
    mutable float m_axisLogTimer = 0.0f;
};

} // namespace ogle