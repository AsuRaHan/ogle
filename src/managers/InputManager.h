// src/managers/InputManager.h
#pragma once

#include <glm/glm.hpp>
#include <array>

namespace ogle {

class Camera;  // Предварительное объявление

class InputManager {
public:
    static InputManager& Get();

    // Обработка клавиши
    void ProcessKey(int keyCode);

    // Обновление с передачей состояния ввода
    void Update(float deltaTime,
                const std::array<bool, 256>& keyStates,
                const glm::vec2& mouseDelta,
                float mouseWheelDelta,
                bool rightMouseDown);

    // Инициализация
    void Initialize();

private:
    InputManager() = default;
    ~InputManager() = default;

    // Запрет копирования
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
};

} // namespace ogle