// src/test/TestCube.cpp
#include "TestCube.h"


namespace ogle {

TestCube::TestCube() = default;

TestCube::~TestCube() {
    Shutdown();
}

bool TestCube::Initialize() {
    Logger::Info("=== TestCube Initialization (Texture Test) ===");

    CreateGeometry();

    // ИСПОЛЬЗУЕМ BasicTexture шейдер вместо BasicColor!
    auto& shaderCtrl = ShaderController::Get();
    m_shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture);

    if (!m_shader) {
        Logger::Error("Failed to get BasicTexture shader");
        // Fallback на BasicColor
        m_shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
    }

    // Тестируем текстуры
    TestTextureLoading();

    return true;
}

void TestCube::TestTextureLoading() {
    auto& texCtrl = TextureController::Get();

    // 1. Встроенные текстуры (уже работают)
    Texture* checker = texCtrl.GetBuiltin(TextureController::Builtin::Checkerboard);

    // 2. Пробуем загрузить реальную текстуру из файла
    std::string texturePath = "../res/koshka-1.png"; // или .jpg

    if (std::filesystem::exists(texturePath)) {
        Texture* realTexture = texCtrl.LoadTexture2D(texturePath, "RealTexture");
        if (realTexture) {
            Logger::Success("REAL texture loaded: " + realTexture->GetName() +
                " (" + std::to_string(realTexture->GetWidth()) +
                "x" + std::to_string(realTexture->GetHeight()) + ")");

            // Переключаемся на реальную текстуру!
            m_currentTexture = realTexture;
        }
        else {
            Logger::Warning("Failed to load real texture, using checkerboard");
            m_currentTexture = checker;
        }
    }
    else {
        Logger::Warning("Texture file not found: " + texturePath);
        Logger::Info("Create 'res/textures/' folder with test_texture.png");
        m_currentTexture = checker;
    }
}

void TestCube::CreateGeometry() {
    // Позиция (3) + Нормаль (3) + Текстурные координаты (2) = 8 floats на вершину
    float vertices[] = {
        // Позиция X,Y,Z, Нормаль X,Y,Z, Текстурные координаты U,V
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
        0,1,2,2,3,0,     // перед
        4,5,6,6,7,4,     // зад
        8,9,10,10,11,8,  // верх
        12,13,14,14,15,12,// низ
        16,17,18,18,19,16,// право
        20,21,22,22,23,20 // лево
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

void TestCube::CreateSimpleShader() {
    // Используем самый простой builtin шейдер
    auto& shaderCtrl = ShaderController::Get();
    m_shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
    
    if (!m_shader) {
        Logger::Error("Failed to get BasicColor shader");
        return;
    }
    
    Logger::Debug("Shader ready: " + m_shader->GetName());
}

//void TestCube::TestTextureLoading() {
//    Logger::Info("=== Testing Texture Loading ===");
//    
//    auto& texCtrl = TextureController::Get();
//    
//    // 1. Получаем встроенную текстуру
//    m_currentTexture = texCtrl.GetBuiltin(TextureController::Builtin::Checkerboard);
//    
//    if (m_currentTexture) {
//        Logger::Success("Builtin texture loaded: " + m_currentTexture->GetName() + 
//                       " (" + std::to_string(m_currentTexture->GetWidth()) + 
//                       "x" + std::to_string(m_currentTexture->GetHeight()) + ")");
//    } else {
//        Logger::Error("Failed to load builtin texture!");
//    }
//    
//    // 2. Показываем статистику
//    texCtrl.PrintDebugInfo();
//    
//    Logger::Info("Texture test completed");
//}

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
    // Просто вращаем куб
    m_rotationAngle += deltaTime * 1.0f;
}

void TestCube::Render(float time, Camera* camera) {
    if (!m_shader || m_vao == 0 || !camera) return;

    m_shader->Bind();

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_rotationAngle, glm::vec3(0.5f, 1.0f, 0.0f));
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix();

    m_shader->SetMat4("uModel", model);
    m_shader->SetMat4("uView", view);
    m_shader->SetMat4("uProjection", projection);

    // Для BasicTexture шейдера
    if (m_shader->GetName().find("BasicTexture") != std::string::npos) {
        m_shader->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
        m_shader->SetInt("uUseLighting", 1);

        // ПРИВЯЗЫВАЕМ ТЕКСТУРУ!
        if (m_currentTexture) {
            m_currentTexture->Bind(0);
            m_shader->SetInt("uTexture", 0); // Texture unit 0
        }
    }
    // Для BasicColor шейдера (fallback)
    else {
        m_shader->SetVec3("uColor", glm::vec3(0.0f, 1.0f, 0.0f));
        m_shader->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
        m_shader->SetInt("uUseLighting", 1);
    }

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    m_shader->Unbind();
}

} // namespace ogle