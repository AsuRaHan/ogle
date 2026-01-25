// src/test/TestCube.cpp
#include "TestCube.h"
#include "log/Logger.h"
#include "render/ShaderController.h"
#include "input/InputController.h"
#include "managers/CameraManager.h"
#include "render/Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

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
    
    m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);

    if (!m_shaderProgram) {
        Logger::Error("Failed to create shader");
        return false;
    }
    
    CreateGeometry();
    
    // Настройка системы ввода!
    SetupInputActions();
    
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

    // Модельная матрица (вращение куба)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, time * m_testRotationSpeed, glm::vec3(0.5f, 1.0f, 0.0f));

    // Используем переданные матрицы камеры
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

void TestCube::Render(float time) {
    // Fallback на статические матрицы если нужно
    glm::mat4 view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        1280.0f / 720.0f,
        0.1f,
        100.0f
    );
    
    Render(time, view, projection);
}

void TestCube::SetupInputActions() {
    auto& input = InputController::Get();
    
    Logger::Info("=== Setting up Input Actions ===");
    
    // 1. ДЕЙСТВИЕ: Переключение шейдеров
    auto* switchAction = input.CreateAction("SwitchShader", ActionType::Button);
    switchAction->AddKey('F');  // Клавиша F
    switchAction->AddKey(VK_TAB);  // Или Tab
    
    switchAction->OnPressed([this](const ActionState& state) {
        Logger::Info("SwitchShader action pressed!");
        CycleShader();
    });
    
    // 2. ДЕЙСТВИЕ: Включение/выключение управления камерой
    auto* cameraToggleAction = input.CreateAction("ToggleCameraControl", ActionType::Button);
    cameraToggleAction->AddKey(VK_SPACE);  // Пробел
    
    cameraToggleAction->OnPressed([this](const ActionState& state) {
        m_cameraControlEnabled = !m_cameraControlEnabled;
        Logger::Info(m_cameraControlEnabled ? 
            "Camera control ENABLED" : "Camera control DISABLED");
    });
    
    // 3. ДЕЙСТВИЕ: Сброс камеры
    auto* resetCameraAction = input.CreateAction("ResetCamera", ActionType::Button);
    resetCameraAction->AddKey('R');
    
    resetCameraAction->OnPressed([](const ActionState& state) {
        Logger::Info("ResetCamera action pressed!");
        auto& cameraMgr = CameraManager::Get();
        Camera* camera = cameraMgr.GetMainCamera();
        if (camera) {
            camera->SetPosition({ 2.0f, 2.0f, 3.0f });
            camera->LookAt({ 0.0f, 0.0f, 0.0f });
            camera->SetMode(Camera::Mode::Free);
        }
    });
    
    // 4. ДЕЙСТВИЕ: Изменение скорости вращения (ось)
    auto* rotationSpeedAction = input.CreateAction("RotationSpeed", ActionType::Axis);
    rotationSpeedAction->AddAxisPair('N', 'M');  // N - медленнее, M - быстрее
    
    rotationSpeedAction->OnHeld([this](const ActionState& state) {
        float speedDelta = state.value * 0.1f; // Меньше изменения
        m_testRotationSpeed += speedDelta;
        // Ограничиваем диапазон
        if (m_testRotationSpeed < 0.1f) m_testRotationSpeed = 0.1f;
        if (m_testRotationSpeed > 5.0f) m_testRotationSpeed = 5.0f;
    });
    
    // 5. ДЕЙСТВИЕ: Вращение куба стрелками
    auto* rotateCubeAction = input.CreateAction("RotateCube", ActionType::Vector2);
    // Используем стрелки для вращения
    rotateCubeAction->AddKey(VK_LEFT, Modifiers{}, -1.0f);  // Влево
    rotateCubeAction->AddKey(VK_RIGHT, Modifiers{}, 1.0f);  // Вправо
    rotateCubeAction->AddKey(VK_UP, Modifiers{}, 1.0f);     // Вверх
    rotateCubeAction->AddKey(VK_DOWN, Modifiers{}, -1.0f);  // Вниз
    
    Logger::Info("Input actions setup complete");
}

void TestCube::Update(float deltaTime) {
    auto& input = InputController::Get();
    // ПРЯМАЯ ПРОВЕРКА КЛАВИШ
    if (input.IsKeyPressed(VK_SPACE)) {
        Logger::Info("=== SPACE pressed directly! ===");
    }

    if (input.IsKeyPressed('R')) {
        Logger::Info("=== R pressed directly! ===");
    }

    if (input.IsKeyPressed('N')) {
        Logger::Info("=== N pressed directly! ===");
    }

    if (input.IsKeyPressed('M')) {
        Logger::Info("=== M pressed directly! ===");
    }
    // === ТЕСТИРОВАНИЕ ОСЕЙ ===
    
    // 1. Получаем значения стандартных осей
    float horizontal = input.GetAxis("Horizontal");
    float vertical = input.GetAxis("Vertical");
    float mouseX = input.GetAxis("MouseX");
    float mouseY = input.GetAxis("MouseY");
    
    // Логируем значения осей (для отладки)
    m_axisLogTimer += deltaTime;
    //if (m_axisLogTimer > 2.0f) {  // Каждые 2 секунды
    //    Logger::Debug("Axes - H: " + std::to_string(horizontal) + 
    //                 " V: " + std::to_string(vertical) +
    //                 " MX: " + std::to_string(mouseX) +
    //                 " MY: " + std::to_string(mouseY));
    //    m_axisLogTimer = 0.0f;
    //}
    
    // 2. Тестируем кастомные действия
    auto* rotationSpeedAction = input.GetAction("RotationSpeed");
    if (rotationSpeedAction) {
        float speedDelta = rotationSpeedAction->GetState().value * deltaTime * 0.5f;
        m_testRotationSpeed += speedDelta;
        // Ограничиваем
        if (m_testRotationSpeed < 0.1f) m_testRotationSpeed = 0.1f;
        if (m_testRotationSpeed > 5.0f) m_testRotationSpeed = 5.0f;
    }
    
    // 3. Управление камерой (если включено)
    if (m_cameraControlEnabled) {
        ProcessCameraInput(deltaTime);
    }
    
    // 4. Проверка клавиш напрямую (для сравнения)
    if (input.IsKeyPressed('T')) {
        Logger::Info("T key pressed directly!");
    }
}

void TestCube::ProcessCameraInput(float deltaTime) {
    auto& input = InputController::Get();
    auto& cameraMgr = CameraManager::Get();
    Camera* camera = cameraMgr.GetMainCamera();
    
    if (!camera) return;
    

    // ДОБАВЬ ОТЛАДКУ
    //Logger::Debug("Camera control: " + std::string(m_cameraControlEnabled ? "ENABLED" : "DISABLED"));
    //Logger::Debug("Right mouse: " + std::string(input.IsMouseButtonDown(1) ? "DOWN" : "UP"));
    //Logger::Debug("Mouse delta: " +
    //    std::to_string(input.GetMouseDelta().x) + ", " +
    //    std::to_string(input.GetMouseDelta().y));


    // 1. Движение через оси
    float moveHorizontal = input.GetAxis("Horizontal");
    float moveVertical = input.GetAxis("Vertical");
    
    // Q/E или Space/Ctrl для движения вверх/вниз
    float moveUp = 0.0f;
    if (input.IsKeyDown('Q') || input.IsKeyDown(VK_SPACE)) moveUp += 1.0f;
    if (input.IsKeyDown('E') || input.IsKeyDown(VK_CONTROL)) moveUp -= 1.0f;
    
    // Применяем движение
    float moveSpeed = 5.0f;
    camera->MoveRight(moveHorizontal * moveSpeed * deltaTime);
    camera->MoveForward(moveVertical * moveSpeed * deltaTime);
    camera->MoveUp(moveUp * moveSpeed * deltaTime);
    
    // 2. Вращение через мышь (при зажатой правой кнопке)
    bool rightMouseDown = input.IsMouseButtonDown(1);
    
    if (rightMouseDown && !m_rightMouseDown) {
        // Запоминаем позицию мыши
        m_lastMousePos = input.GetMousePosition();
    }
    m_rightMouseDown = rightMouseDown;
    
    if (rightMouseDown) {
        // Получаем дельту через оси мыши
        float mouseX = input.GetAxis("MouseX");
        float mouseY = input.GetAxis("MouseY");
        
        if (std::abs(mouseX) > 0.01f || std::abs(mouseY) > 0.01f) {
            camera->ProcessMouseMovement(mouseX * 0.5f, mouseY * 0.5f);
        }
    }
    
    // 3. Zoom через колесико
    float mouseWheel = input.GetMouseWheelDelta();
    if (std::abs(mouseWheel) > 0.01f) {
        camera->ProcessMouseScroll(mouseWheel);
    }
}

void TestCube::CycleShader() {
    auto& shaderCtrl = ShaderController::Get();

    static int currentShader = 0;
    currentShader = (currentShader + 1) % 4; // Только 4 шейдера для куба!

    switch (currentShader) {
    case 0:
        m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
        Logger::Info("Switched to: BasicColor shader");
        break;
    case 1:
        m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture);
        Logger::Info("Switched to: BasicTexture shader");
        break;
    case 2:
        m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::Unlit);
        Logger::Info("Switched to: Unlit shader");
        break;
    case 3:
        m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::Wireframe);
        Logger::Info("Switched to: Wireframe shader");
        break;
        // Skybox убран - он не для кубов!
    }
}

} // namespace ogle