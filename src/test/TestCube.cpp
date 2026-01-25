#include "TestCube.h"
#include "log/Logger.h"
#include "render/ShaderController.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "input/InputController.h"  // Для доступа к состоянию ввода

namespace ogle {

TestCube::TestCube() = default;

TestCube::~TestCube() {
    Shutdown();
}

bool TestCube::Initialize() {
    Logger::Info("TestCube initializing...");
    
    // Сначала проверим все builtin шейдеры
    TestBuiltinShaders();

    auto& shaderCtrl = ShaderController::Get();
    
    m_shaderProgram = shaderCtrl.CreateProgram("ColorfulCube",
        R"(#version 460 core
           layout(location = 0) in vec3 aPos;
           layout(location = 1) in vec3 aColor;
           
           uniform mat4 uModel;
           uniform mat4 uView;
           uniform mat4 uProjection;
           
           out vec3 vColor;
           
           void main() {
               vColor = aColor;
               gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
           })",
        
        R"(#version 460 core
           in vec3 vColor;
           out vec4 FragColor;
           
           void main() {
               FragColor = vec4(vColor, 1.0);
           })"
    );
    
    //m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);

    if (!m_shaderProgram) {
        Logger::Error("Failed to create shader");
        return false;
    }
    
    CreateGeometry();
    Logger::Info("TestCube initialized");
    return true;
}

bool TestCube::TestBuiltinShaders() {
    Logger::Info("=== Testing Builtin Shaders ===");

    auto& shaderCtrl = ShaderController::Get();

    // Тестируем каждый builtin шейдер
    bool allPassed = true;

    // 1. BasicColor
    auto basicColor = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
    if (!basicColor) {
        Logger::Error("BasicColor shader failed");
        allPassed = false;
    }
    else {
        Logger::Info("✓ BasicColor: OK (ID: " + std::to_string(basicColor->GetID()) + ")");
    }

    // 2. BasicTexture  
    auto basicTexture = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture);
    if (!basicTexture) {
        Logger::Error("BasicTexture shader failed");
        allPassed = false;
    }
    else {
        Logger::Info("✓ BasicTexture: OK (ID: " + std::to_string(basicTexture->GetID()) + ")");
    }

    // 3. Skybox
    auto skybox = shaderCtrl.GetBuiltin(ShaderController::Builtin::Skybox);
    if (!skybox) {
        Logger::Error("Skybox shader failed");
        allPassed = false;
    }
    else {
        Logger::Info("✓ Skybox: OK (ID: " + std::to_string(skybox->GetID()) + ")");
    }

    // 4. Unlit
    auto unlit = shaderCtrl.GetBuiltin(ShaderController::Builtin::Unlit);
    if (!unlit) {
        Logger::Error("Unlit shader failed");
        allPassed = false;
    }
    else {
        Logger::Info("✓ Unlit: OK (ID: " + std::to_string(unlit->GetID()) + ")");
    }

    // 5. Wireframe
    auto wireframe = shaderCtrl.GetBuiltin(ShaderController::Builtin::Wireframe);
    if (!wireframe) {
        Logger::Error("Wireframe shader failed");
        allPassed = false;
    }
    else {
        Logger::Info("✓ Wireframe: OK (ID: " + std::to_string(wireframe->GetID()) + ")");
    }

    if (allPassed) {
        Logger::Info("=== All builtin shaders PASSED ===");
    }
    else {
        Logger::Error("=== Some builtin shaders FAILED ===");
    }

    return allPassed;
}

void TestCube::SetShaderMode(ShaderMode mode) {
    auto& shaderCtrl = ShaderController::Get();

    switch (mode) {
    case ShaderMode::Colorful:
        // Используем существующий шейдер
        Logger::Info("Switched to Colorful shader");
        break;

    case ShaderMode::BasicColor:
        m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
        Logger::Info("Switched to BasicColor shader");
        break;

    case ShaderMode::BasicTexture:
        m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture);
        Logger::Info("Switched to BasicTexture shader");
        break;

    case ShaderMode::Unlit:
        m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::Unlit);
        Logger::Info("Switched to Unlit shader");
        break;

    case ShaderMode::Wireframe:
        m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::Wireframe);
        Logger::Info("Switched to Wireframe shader");
        break;
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
    
    m_shaderProgram.reset();
    Logger::Debug("TestCube shutdown");
}

void TestCube::CreateGeometry() {
    float vertices[] = {
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
        
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0,1,2,2,3,0, 4,5,6,6,7,4,
        8,9,10,10,11,8, 12,13,14,14,15,12,
        16,17,18,18,19,16, 20,21,22,22,23,20
    };
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TestCube::Render(float time, const glm::mat4& view, const glm::mat4& projection) {
    if (!m_shaderProgram || m_vao == 0) return;

    m_shaderProgram->Bind();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, time, glm::vec3(0.5f, 1.0f, 0.0f));


    // Общие uniform для всех шейдеров
    m_shaderProgram->SetMat4("uModel", model);
    m_shaderProgram->SetMat4("uView", view);
    m_shaderProgram->SetMat4("uProjection", projection);

    // Настройки для разных типов шейдеров
    if (m_shaderProgram->GetName().find("BasicColor") != std::string::npos) {
        m_shaderProgram->SetVec3("uColor", glm::vec3(0.2f, 0.8f, 0.2f)); // Зеленый
        m_shaderProgram->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
        m_shaderProgram->SetInt("uUseLighting", 1);
    }
    else if (m_shaderProgram->GetName().find("Unlit") != std::string::npos) {
        m_shaderProgram->SetVec3("uColor", glm::vec3(0.8f, 0.2f, 0.8f)); // Пурпурный
        m_shaderProgram->SetInt("uUseTexture", 0); // Без текстуры
    }
    else if (m_shaderProgram->GetName().find("Wireframe") != std::string::npos) {
        m_shaderProgram->SetVec3("uColor", glm::vec3(1.0f, 1.0f, 1.0f)); // Белый
        m_shaderProgram->SetInt("uShowNormals", 0); // Не показывать нормали
    }
    else if (m_shaderProgram->GetName().find("BasicTexture") != std::string::npos) {
        m_shaderProgram->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
        m_shaderProgram->SetInt("uUseLighting", 1);
        // Текстуры пока нет, будет цвет по умолчанию
    }

    // Рисуем
    glBindVertexArray(m_vao);

    // Для Wireframe рисуем линиями
    if (m_shaderProgram->GetName().find("Wireframe") != std::string::npos) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else {
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    m_shaderProgram->Unbind();
}


void TestCube::SetupInputActions() {
    auto& input = InputController::Get();

    // Создаем действие "SwitchShader"
    auto* switchAction = input.CreateAction("SwitchShader", ActionType::Button);
    switchAction->AddKey('F');  // Клавиша F

    // Подписываемся на событие
    switchAction->OnPressed([](const ActionState& state) {
        Logger::Info("F pressed - switching shader!");

        // Здесь можно переключать шейдеры
        // Например: SwitchToNextShader();
        });

    // Создаем действие "RotateSpeed"
    auto* speedAction = input.CreateAction("RotateSpeed", ActionType::Axis);
    speedAction->AddAxisPair('N', 'M');  // N - медленнее, M - быстрее
    speedAction->AddGamepadAxis(0, GamepadAxis::RightTrigger, 0.1f, 2.0f);

    // Или более сложное действие
    auto* zoomAction = input.CreateAction("Zoom", ActionType::Axis);
    zoomAction->AddGamepadAxis(0, GamepadAxis::RightStickY, 0.1f, 0.5f);

    Logger::Info("Input actions setup complete");
}


} // namespace ogle