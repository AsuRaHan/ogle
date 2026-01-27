// src/test/TestCube.cpp
#include "TestCube.h"
#include "log/Logger.h"
#include "render/ShaderController.h"
#include "render/TextureController.h"
#include "render/MaterialController.h"
#include "render/material/Material.h"
#include "render/Camera.h"
#include "input/InputController.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ogle {

    TestCube::TestCube() = default;

    TestCube::~TestCube() {
        Shutdown();
    }

    bool TestCube::Initialize() {
        Logger::Info("=== TestCube Initialization ===");

        // 1. Создаем геометрию куба
        CreateGeometry();

        // 2. Тестируем текстуры
        TestTextureSystem();

        // 3. Тестируем материалы
        TestMaterialSystem();

        // 4. Создаем тестовые материалы
        CreateTestMaterials();

        Logger::Success("TestCube initialized");
        return true;
    }

    void TestCube::CreateGeometry() {
        // Простой куб с позициями и нормалями
        float vertices[] = {
            // Позиция X,Y,Z, Нормаль X,Y,Z
            // Передняя грань
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

            // Задняя грань  
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
        };

        unsigned int indices[] = {
            0,1,2,2,3,0,     // перед
            4,5,6,6,7,4,     // зад
            3,2,6,6,7,3,     // верх
            0,1,5,5,4,0,     // низ
            1,5,6,6,2,1,     // право
            0,4,7,7,3,0      // лево
        };

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Атрибут 0: Позиция (3 float)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Атрибут 1: Нормаль (3 float)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        Logger::Debug("Cube geometry created");
    }

    void TestCube::TestTextureSystem() {
        Logger::Info("=== Testing Texture System ===");

        auto& texCtrl = TextureController::Get();
        texCtrl.PrintDebugInfo();

        Logger::Info("Texture test completed");
    }

// В тесте используйте dynamic_cast для доступа к специфичным методам:
void TestCube::TestMaterialSystem() {
    Logger::Info("=== Testing Material System ===");

    auto& matCtrl = MaterialController::Get();

    // Простой тест: создаем и сразу удаляем материал
    auto* testMat = matCtrl.CreateMaterial("SystemTest", MaterialType::Basic);
    if (testMat && testMat->GetType() == MaterialType::Basic) {
        Logger::Success("✓ Material creation works");

        // Приводим к BasicMaterial
        auto* basicMat = dynamic_cast<BasicMaterial*>(testMat);
        if (basicMat) {
            basicMat->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            auto color = basicMat->GetColor();

            if (color.r == 1.0f && color.g == 0.0f && color.b == 0.0f) {
                Logger::Success("✓ Material parameters work");
            }
        }
        
        // Очищаем тестовый материал
        matCtrl.RemoveMaterial("SystemTest");
    }
}

void TestCube::CreateTestMaterials() {
    Logger::Info("=== Creating Test Materials ===");

    auto& matCtrl = MaterialController::Get();
    auto& texCtrl = TextureController::Get();
    auto& shaderCtrl = ShaderController::Get();

    // 1. Красный материал
    auto* redMat = matCtrl.CreateMaterial("TestRed", MaterialType::Basic);
    if (redMat) {
        redMat->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        // ВАЖНО: BasicColor шейдер для цветных материалов
        redMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
        redMat->SetUseLighting(true);
        redMat->SetInt("uUseTexture", 0); // Явно отключаем текстуру
        m_materials.push_back(redMat);
        Logger::Info("Created: Red material");
    }

    // 2. Зеленый материал
    auto* greenMat = matCtrl.CreateMaterial("TestGreen", MaterialType::Basic);
    if (greenMat) {
        greenMat->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        greenMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
        greenMat->SetUseLighting(true);
        greenMat->SetInt("uUseTexture", 0);
        m_materials.push_back(greenMat);
        Logger::Info("Created: Green material");
    }

    // 3. Синий материал
    auto* blueMat = matCtrl.CreateMaterial("TestBlue", MaterialType::Basic);
    if (blueMat) {
        blueMat->SetColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        blueMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
        blueMat->SetUseLighting(true);
        blueMat->SetInt("uUseTexture", 0);
        m_materials.push_back(blueMat);
        Logger::Info("Created: Blue material");
    }

    // 4. Текстурированный материал - ВАЖНО: используем BasicTexture шейдер!
    auto* texMat = matCtrl.CreateMaterial("TestTextured", MaterialType::Basic);
    if (texMat) {
        Texture* checker = texCtrl.GetBuiltin(TextureController::Builtin::Checkerboard);
        texMat->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        texMat->SetTexture(checker);
        // BasicTexture шейдер для текстурированных материалов
        texMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture).get());
        texMat->SetUseLighting(true);
        texMat->SetInt("uUseColor", 0); // Используем текстуру, не цвет
        m_materials.push_back(texMat);
        Logger::Info("Created: Textured material");
    }

    // 5. Wireframe материал
    auto* wireMat = matCtrl.CreateMaterial("TestWireframe", MaterialType::Basic);
    if (wireMat) {
        wireMat->SetColor(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
        wireMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
        wireMat->SetUseLighting(false);
        wireMat->SetInt("uUseTexture", 0);
        wireMat->GetRenderState().wireframe = true;
        wireMat->GetRenderState().lineWidth = 2.0f;
        m_materials.push_back(wireMat);
        Logger::Info("Created: Wireframe material");
    }
    // Устанавливаем первый материал как текущий
    if (!m_materials.empty()) {
        m_currentMaterial = m_materials[0];
        m_currentMaterialIndex = 0;
        Logger::Info("Current material: " + m_currentMaterial->GetName());
    }

    Logger::Info("Created " + std::to_string(m_materials.size()) + " test materials");
}

    void TestCube::LogMaterialInfo(Material* material) {
        if (!material) return;

        Logger::Debug("Material: " + material->GetName());

        // Пробуем получить как BasicMaterial
        if (auto* basicMat = dynamic_cast<BasicMaterial*>(material)) {
            auto color = basicMat->GetColor();
            Logger::Debug("  Type: BasicMaterial");
            Logger::Debug("  Color: " +
                std::to_string(color.r) + ", " +
                std::to_string(color.g) + ", " +
                std::to_string(color.b));
            Logger::Debug("  UseLighting: " + std::string(basicMat->GetUseLighting() ? "Yes" : "No"));

            if (basicMat->GetTexture()) {
                Logger::Debug("  Texture: " + basicMat->GetTexture()->GetName());
            }
        }
    }

    void TestCube::Shutdown() {
        if (m_vao != 0) {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_vbo != 0) {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_ebo != 0) {
            glDeleteBuffers(1, &m_ebo);
            m_ebo = 0;
        }

        Logger::Debug("TestCube shutdown");
    }

    void TestCube::Update(float deltaTime) {
        m_rotationAngle += deltaTime * 1.0f;

        // Автоматическое переключение материалов каждые 3 секунды
        static float switchTimer = 0.0f;
        switchTimer += deltaTime;

        if (switchTimer > 5.0f && m_materials.size() > 1) {
            SwitchToNextMaterial();
            switchTimer = 0.0f;
        }
    }

    void TestCube::Render(float time, Camera* camera) {
        if (m_vao == 0 || !camera) return;

        // Если есть текущий материал - используем его
    // Если есть текущий материал - используем его
    if (m_currentMaterial && m_currentMaterial->GetShader()) {
        auto* shader = m_currentMaterial->GetShader();
        shader->Bind();

        // Устанавливаем матрицы
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_rotationAngle,
            glm::vec3(0.5f, 1.0f, 0.0f));
        shader->SetMat4("uModel", model);
        shader->SetMat4("uView", camera->GetViewMatrix());
        shader->SetMat4("uProjection", camera->GetProjectionMatrix());
        
        // ВАЖНО: Устанавливаем направление света
        shader->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));

        // Применяем материал
        m_currentMaterial->Apply(shader);

        // Применяем состояние рендеринга
        m_currentMaterial->GetRenderState().Apply();

    }
        else {
            // Fallback: используем BasicColor шейдер
            auto& shaderCtrl = ShaderController::Get();
            auto shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
            if (!shader) return;

            shader->Bind();

            glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_rotationAngle,
                glm::vec3(0.5f, 1.0f, 0.0f));
            shader->SetMat4("uModel", model);
            shader->SetMat4("uView", camera->GetViewMatrix());
            shader->SetMat4("uProjection", camera->GetProjectionMatrix());
            shader->SetVec3("uColor", glm::vec3(1.0f, 0.0f, 0.0f)); // Красный
            shader->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
            shader->SetInt("uUseLighting", 1);
        }

        // Рисуем куб
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Отключаем шейдер
        if (m_currentMaterial && m_currentMaterial->GetShader()) {
            m_currentMaterial->GetShader()->Unbind();
        }
        else {
            ShaderController::Get().GetCurrentProgram()->Unbind();
        }

        // Восстанавливаем режим полигонов
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void TestCube::SwitchToNextMaterial() {
        if (m_materials.empty()) return;

        m_currentMaterialIndex = (m_currentMaterialIndex + 1) % m_materials.size();
        m_currentMaterial = m_materials[m_currentMaterialIndex];

        if (m_currentMaterial) {
            Logger::Info("=== Material Switched ===");
            Logger::Info("Material [" + std::to_string(m_currentMaterialIndex + 1) +
                "/" + std::to_string(m_materials.size()) + "]: " +
                m_currentMaterial->GetName());

            LogMaterialInfo(m_currentMaterial);
        }
    }

} // namespace ogle