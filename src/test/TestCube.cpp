// src/test/TestCube.cpp
#include "TestCube.h"

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

		// 5. Запускаем тест сериализации материалов
        //TestMaterialSerialization();

        Logger::Success("TestCube initialized");
        return true;
    }

    void TestCube::CreateGeometry() {
        // Позиция (3) + Нормаль (3) + Текстурные координаты (2) = 8 floats на вершину
        float vertices[] = {
            // Передняя грань
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

            // Задняя грань
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,

            // Верхняя грань
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

            // Нижняя грань
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,

            // Правая грань
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

             // Левая грань
             -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
             -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
             -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
             -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        };


        // Индексы те же
        unsigned int indices[] = {
            0, 1, 2, 2, 3, 0,     // передняя грань
            4, 5, 6, 6, 7, 4,     // задняя грань
            8, 9, 10, 10, 11, 8,  // верхняя грань
            12, 13, 14, 14, 15, 12, // нижняя грань
            16, 17, 18, 18, 19, 16, // правая грань
            20, 21, 22, 22, 23, 20 // левая грань
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Атрибут 1: Нормаль (3 float)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Атрибут 2: Текстурные координаты (2 float)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        Logger::Debug("Cube geometry with normals and UVs created");
    }

    void TestCube::TestTextureSystem() {
        Logger::Info("=== Testing Texture System ===");

        auto& texCtrl = TextureController::Get();
        texCtrl.PrintDebugInfo();

        // Example: load a real texture from file (adjust path as needed)
        std::string texturePath = "../res/koshka-1.png"; // example path, change to your file
        auto* loaded = texCtrl.LoadTexture2D(texturePath, "Test_Koshka");
        if (loaded) {
            Logger::Success("✓ Texture loaded: " + loaded->GetName());
        } else {
            Logger::Warning("Texture not loaded from: " + texturePath + " (check path)");
        }

        // Test builtin textures
        auto* white = texCtrl.GetBuiltin(TextureController::Builtin::White1x1);
        if (white) Logger::Success("✓ Builtin white texture available: " + white->GetName());

        // Enable watching for hot-reload (optional)
        texCtrl.WatchTextureFiles(true);
        texCtrl.CheckForUpdates();

        texCtrl.PrintDebugInfo();

        Logger::Info("Texture test completed");
    }

    // В тесте используйте dynamic_cast для доступа к специфичным методам:
    void TestCube::TestMaterialSystem() {
        Logger::Info("=== Testing Material System ===");

        auto& matCtrl = MaterialController::Get();

        auto& texCtrl = TextureController::Get();
        auto& shaderCtrl = ShaderController::Get();

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

        // Дополнительная проверка: создание материалла с текстурой и шейдером
        std::string texturePath = "../res/koshka-1.png"; // example path
        Texture* tstTex = texCtrl.LoadTexture2D(texturePath, "MatTestTexture");

        auto* mat = matCtrl.CreateMaterial("MatTest", MaterialType::Basic);
        if (mat) {
            // Use BasicTexture shader for textured materials
            auto shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture);
            if (shader) mat->SetShader(shader.get());

            if (tstTex) {
                mat->SetTexture(tstTex);
                mat->SetInt("uUseColor", 0); // use texture
                Logger::Success("✓ Material assigned texture: " + tstTex->GetName());
            }

            mat->SetColor(glm::vec4(1.0f));
            mat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));

            // Create instance
            auto* instance = matCtrl.CreateInstance(mat, "MatTest_Instance");
            if (instance) Logger::Success("✓ Material instance created: " + instance->GetName());

            // Save and load material to test serialization (paths are examples)
            std::string savePath = "../res/test_material.json";
            if (matCtrl.SaveMaterialToFile(mat->GetName(), savePath)) {
                Logger::Success("✓ Material saved to: " + savePath);
                auto* loaded = matCtrl.LoadMaterialFromFile(savePath, "LoadedMatTest");
                if (loaded) Logger::Success("✓ Material loaded from file: " + loaded->GetName());
                else Logger::Warning("Material load failed from: " + savePath);
            } else {
                Logger::Warning("Material save failed: " + savePath);
            }
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
            redMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
            redMat->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            redMat->SetUseLighting(true);
            redMat->SetInt("uUseTexture", 0);  // ← ТОЛЬКО для BasicColor!
            redMat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
            m_materials.push_back(redMat);
            Logger::Info("TestCube Created: Red material");
        }

        // 2. Зеленый материал
        auto* greenMat = matCtrl.CreateMaterial("TestGreen", MaterialType::Basic);
        if (greenMat) {
            greenMat->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            greenMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
            greenMat->SetUseLighting(true);
            greenMat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
            greenMat->SetInt("uUseTexture", 0);
            m_materials.push_back(greenMat);
            Logger::Info("TestCube Created: Green material");
        }

        // 3. Синий материал
        auto* blueMat = matCtrl.CreateMaterial("TestBlue", MaterialType::Basic);
        if (blueMat) {
            blueMat->SetColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            blueMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
            blueMat->SetUseLighting(true);
            blueMat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
            blueMat->SetInt("uUseTexture", 0);
            m_materials.push_back(blueMat);
            Logger::Info("TestCube Created: Blue material");
        }

        // 4. Текстурированный материал
    // 4. Текстурированный материал - используем BasicTexture шейдер!
        auto* texMat = matCtrl.CreateMaterial("TestTextured", MaterialType::Basic);
        if (texMat) {
            Texture* checker = texCtrl.GetBuiltin(TextureController::Builtin::Checkerboard);

            // 1. Сначала устанавливаем шейдер
            texMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture).get());

            // 2. Устанавливаем цвет (белый)
            texMat->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            // 3. Устанавливаем текстуру (ВАЖНО: в BasicTexture шейдере SetTexture должен установить uUseColor = 0)
            texMat->SetTexture(checker);

            // 4. Дополнительные параметры
            texMat->SetInt("uUseLighting", 1);
            texMat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));

            // 5. ЯВНО указываем что использовать текстуру, а не цвет
            texMat->SetInt("uUseColor", 0);  // ← 0 = использовать текстуру, 1 = использовать цвет

            m_materials.push_back(texMat);
            Logger::Info("Created: Textured material");
        }

        // 5. Wireframe материал
        auto* wireMat = matCtrl.CreateMaterial("TestWireframe", MaterialType::Basic);
        if (wireMat) {
            wireMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
            wireMat->SetColor(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
            wireMat->SetUseLighting(false);  // Wireframe обычно без освещения
            wireMat->SetInt("uUseTexture", 0);
            wireMat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
            wireMat->GetRenderState().wireframe = true;
            wireMat->GetRenderState().lineWidth = 2.0f;
            m_materials.push_back(wireMat);
            Logger::Info("TestCube Created: Wireframe material");
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
        if (m_currentMaterial && m_currentMaterial->GetShader()) {
            auto* shader = m_currentMaterial->GetShader();
            shader->Bind();

            // 1. Устанавливаем матрицы
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_rotationAngle,
                glm::vec3(0.5f, 1.0f, 0.0f));
            shader->SetMat4("uModel", model);
            shader->SetMat4("uView", camera->GetViewMatrix());
            shader->SetMat4("uProjection", camera->GetProjectionMatrix());

            // 2. Устанавливаем направление света В МАТЕРИАЛ, а не напрямую в шейдер!
            m_currentMaterial->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));

            // 3. Применяем материал (он установит все uniform'ы включая uLightDir)
            m_currentMaterial->Apply(shader);

            // 4. Применяем состояние рендеринга
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