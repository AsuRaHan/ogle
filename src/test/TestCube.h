// src/test/TestCube.h
#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include "render/Camera.h"

// В начале TestCube.cpp добавьте:
#include "render/shader/ShaderProgram.h"      // Для ShaderProgram
#include "render/texture/Texture.h"           // Для Texture
#include "render/ShaderController.h"          // Для ShaderController
#include "render/TextureController.h"         // Для TextureController
#include "log/Logger.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace ogle {

// Предварительные объявления
class ShaderProgram;
class Texture;

class TestCube {
public:
    TestCube();
    ~TestCube();
    
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    void Render(float time, Camera* camera);
    
    // Только для теста текстур
    void TestTextureLoading();
    
private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    
    std::shared_ptr<ShaderProgram> m_shader;
    Texture* m_currentTexture = nullptr;
    
    float m_rotationAngle = 0.0f;
    
    void CreateGeometry();
    void CreateSimpleShader();
};

} // namespace ogle