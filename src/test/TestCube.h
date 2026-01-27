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
    class Material;
    class Camera;

    class TestCube {
    public:
        TestCube();
        ~TestCube();

        bool Initialize();
        void Shutdown();

        void Update(float deltaTime);
        void Render(float time, Camera* camera);

        // Тесты
        void TestTextureSystem();
        void TestMaterialSystem();

        // Управление материалами
        void SwitchToNextMaterial();

    private:
        // OpenGL объекты
        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ebo = 0;

        // Тестовые материалы
        std::vector<Material*> m_materials;
        Material* m_currentMaterial = nullptr;
        size_t m_currentMaterialIndex = 0;

        // Параметры
        float m_rotationAngle = 0.0f;

        // Вспомогательные методы
        void CreateGeometry();
        void CreateTestMaterials();
        void LogMaterialInfo(Material* material);
    };

} // namespace ogle