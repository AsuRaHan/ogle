// src/managers/InputManager.cpp
#include "InputManager.h"
#include "log/Logger.h"
#include "managers/CameraManager.h"
#include "render/Camera.h"
#include <windows.h>

namespace ogle {

InputManager& InputManager::Get() {
    static InputManager instance;
    return instance;
}

void InputManager::ProcessKey(int keyCode) {
    if (keyCode == VK_ESCAPE) {
        PostQuitMessage(0);
        Logger::Info("Escape pressed - quitting application");
    }
}

void InputManager::Initialize() {
    Logger::Info("InputManager initialized");
}

void InputManager::Update(float deltaTime,
                         const std::array<bool, 256>& keyStates,
                         const glm::vec2& mouseDelta,
                         float mouseWheelDelta,
                         bool rightMouseDown) {
    
    // Получаем камеру через менеджер
    auto& cameraMgr = CameraManager::Get();
    Camera* camera = cameraMgr.GetMainCamera();
    
    if (!camera) {
        return;  // Камеры нет - выходим
    }
    
    // Настраиваем скорость
    float moveSpeed = 5.0f;
    float mouseSensitivity = 0.1f;
    
    // === Управление WSAD (всегда работает) ===
    if (keyStates['W']) {
        camera->MoveForward(moveSpeed * deltaTime);
    }
    if (keyStates['S']) {
        camera->MoveForward(-moveSpeed * deltaTime);
    }
    if (keyStates['A']) {
        camera->MoveRight(-moveSpeed * deltaTime);
    }
    if (keyStates['D']) {
        camera->MoveRight(moveSpeed * deltaTime);
    }
    
    // Q/E - движение вверх/вниз
    if (keyStates['Q']) {
        camera->MoveUp(moveSpeed * deltaTime);
    }
    if (keyStates['E']) {
        camera->MoveUp(-moveSpeed * deltaTime);
    }
    
    // Space/Ctrl - движение вверх/вниз (альтернатива Q/E)
    if (keyStates[VK_SPACE]) {
        camera->MoveUp(moveSpeed * deltaTime);
    }
    if (keyStates[VK_CONTROL]) {
        camera->MoveUp(-moveSpeed * deltaTime);
    }
    
    // R - сброс камеры в начальную позицию
    static bool rWasPressed = false;
    if (keyStates['R'] && !rWasPressed) {
        camera->SetPosition({ 2.0f, 2.0f, 3.0f });
        camera->LookAt({ 0.0f, 0.0f, 0.0f });
        Logger::Info("Camera reset");
        rWasPressed = true;
    } else if (!keyStates['R']) {
        rWasPressed = false;
    }
    
    // === Управление мышкой (только при зажатой правой кнопке) ===
    if (rightMouseDown) {
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
            // Вращаем камеру
            camera->ProcessMouseMovement(
                mouseDelta.x * mouseSensitivity,
                -mouseDelta.y * mouseSensitivity  // Инвертируем Y
            );
        }
    }
    
    // Прокрутка колесика мыши - зум/изменение FOV
    if (mouseWheelDelta != 0.0f) {
        camera->ProcessMouseScroll(mouseWheelDelta);
    }
    
    // F1 - переключение режима камеры (дополнительная фича)
    static bool f1WasPressed = false;
    if (keyStates[VK_F1] && !f1WasPressed) {
        Camera::Mode currentMode = camera->GetMode();
        Camera::Mode newMode;
        
        switch (currentMode) {
            case Camera::Mode::Free:
                newMode = Camera::Mode::FirstPerson;
                Logger::Info("Camera mode: FirstPerson");
                break;
            case Camera::Mode::FirstPerson:
                newMode = Camera::Mode::Orbit;
                Logger::Info("Camera mode: Orbit");
                break;
            case Camera::Mode::Orbit:
                newMode = Camera::Mode::Free;
                Logger::Info("Camera mode: Free");
                break;
            default:
                newMode = Camera::Mode::Free;
        }
        
        camera->SetMode(newMode);
        f1WasPressed = true;
    } else if (!keyStates[VK_F1]) {
        f1WasPressed = false;
    }
}

} // namespace ogle