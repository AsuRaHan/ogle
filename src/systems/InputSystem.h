// src/systems/InputSystem.h
#pragma once

#include <windows.h>
#include <glm/glm.hpp>
#include "core/ISystem.h"
#include "input/InputTypes.h"

namespace ogle {

class InputSystem final : public ISystem {
public:
    InputSystem();
    ~InputSystem() override;

    // === ISystem интерфейс ===
    const std::string& GetName() const override;
    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render() override {}
    void Shutdown() override;
    void OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

    // === Только для Engine ===
    void SetWindowHandle(HWND hwnd);

private:
    // === ОБРАБОТКА СОБЫТИЙ ===
    void ProcessKeyEvent(UINT msg, WPARAM wParam, LPARAM lParam);
    void ProcessMouseEvent(UINT msg, WPARAM wParam, LPARAM lParam);
    void ProcessRawInput(LPARAM lParam);

    // === ОБНОВЛЕНИЕ ===
    void UpdateGamepads();
    void ResetFrameState();
    
    // === ОБНОВЛЕНИЕ КОНТРОЛЛЕРА ===
    void UpdateController();

    // === ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ===
    static float NormalizeAxis(SHORT value);
    static void ApplyDeadzone(float& x, float& y, SHORT deadzone);

    // === ЗАХВАТ МЫШИ (внутренний метод) ===
    void SetMouseCapture(bool capture);

    // === СОСТОЯНИЕ (ВОТ ЭТИ ПОЛЯ ВСЕ НУЖНЫ!) ===
    HWND m_hwnd = nullptr;

    // Клавиатура
    bool m_keyStates[256] = { false };
    bool m_keyPressedThisFrame[256] = { false };
    bool m_keyReleasedThisFrame[256] = { false };

    // Мышь
    glm::vec2 m_mousePosition = { 0.0f, 0.0f };
    glm::vec2 m_mouseDelta = { 0.0f, 0.0f };
    float m_mouseWheelDelta = 0.0f;
    bool m_mouseButtonStates[5] = { false };
    bool m_mousePressedThisFrame[5] = { false };
    bool m_mouseReleasedThisFrame[5] = { false };

    // Геймпады
    struct GamepadState {
        bool connected = false;
        bool buttons[14] = { false };
        float axes[6] = { 0.0f };

        void Reset() {
            connected = false;
            memset(buttons, 0, sizeof(buttons));
            memset(axes, 0, sizeof(axes));
        }
    };
    GamepadState m_gamepads[4];

    // Модификаторы
    Modifiers m_currentModifiers;

    // Захват мыши
    bool m_mouseCaptured = false;
    RECT m_clipRect = {};
    POINT m_lastMousePos = {};
};

} // namespace ogle