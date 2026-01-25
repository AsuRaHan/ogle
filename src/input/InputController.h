// src/input/InputController.h
#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <unordered_map>

#include "input/InputAction.h"
#include "input/InputTypes.h"
#include "log/Logger.h"

// Предварительные объявления
namespace ogle {
    class InputAction;
    enum class ActionType;
    struct Modifiers;
}

namespace ogle {

// Главный контроллер ввода (доступен везде, как ShaderController)
class InputController {
public:
    static InputController& Get();
    
    // === КЛАВИАТУРА (полный аналог методов из InputSystem) ===
    
    // Проверка состояния клавиши
    bool IsKeyDown(int vk) const;          // Клавиша нажата
    bool IsKeyPressed(int vk) const;       // Клавиша нажата в этом кадре
    bool IsKeyReleased(int vk) const;      // Клавиша отпущена в этом кадре
    
    // === МЫШЬ ===
    
    // Позиция мыши
    glm::vec2 GetMousePosition() const;
    
    // Движение мыши (разница с предыдущим кадром)
    glm::vec2 GetMouseDelta() const;
    
    // Колесико мыши
    float GetMouseWheelDelta() const;
    
    // Кнопки мыши
    bool IsMouseButtonDown(int button) const;      // Кнопка нажата
    bool IsMouseButtonPressed(int button) const;   // Кнопка нажата в этом кадре
    bool IsMouseButtonReleased(int button) const;  // Кнопка отпущена в этом кадре
    
    // === ГЕЙМПАДЫ ===
    
    // Подключение
    bool IsGamepadConnected(int player = 0) const;
    
    // Кнопки геймпада
    bool IsGamepadButtonDown(int player, int button) const;
    
    // Оси геймпада
    float GetGamepadAxis(int player, int axis) const;
    
    // === INPUT ACTIONS (из InputSystem) ===
    
    // Создание и управление действиями
    InputAction* CreateAction(const std::string& name, ActionType type);
    InputAction* GetAction(const std::string& name);
    void RemoveAction(const std::string& name);
    
    // === КОНТЕКСТЫ (из InputSystem) ===
    
    // Система контекстов ввода (например, "Menu", "Gameplay")
    void PushContext(const std::string& context);
    void PopContext();
    const std::string& GetCurrentContext() const;
    
    // === МОДИФИКАТОРЫ ===
    
    // Текущие модификаторы (Ctrl, Shift, Alt, Win)
    const Modifiers& GetCurrentModifiers() const;
    
    // === ОБНОВЛЕНИЕ (для InputSystem) ===
    
    // Эти методы будет вызывать InputSystem для обновления состояния
    
    // Обновить состояние клавиатуры
    void UpdateKeyboardState(
        const bool keyStates[256],
        const bool keyPressed[256],
        const bool keyReleased[256]
    );
    
    // Обновить состояние мыши
    void UpdateMouseState(
        const glm::vec2& position,
        const glm::vec2& delta,
        float wheelDelta,
        const bool buttonStates[5],
        const bool buttonPressed[5],
        const bool buttonReleased[5]
    );
    
    // Обновить состояние геймпада
    void UpdateGamepadState(
        int player,
        bool connected,
        const bool buttons[14],
        const float axes[6]
    );
    
    // Обновить модификаторы
    void UpdateModifiers(const Modifiers& modifiers);
    
    // Обновить все действия (вызывать каждый кадр)
    void UpdateActions(float deltaTime);
    
    // Сбросить состояние кадра (вызывать в конце кадра)
    void ResetFrameState();
    
    // === УТИЛИТЫ ===
    
    // Количество зарегистрированных действий
    size_t GetActionCount() const { return m_actions.size(); }
    
    // Проверка, есть ли действие с таким именем
    bool HasAction(const std::string& name) const;
    
private:
    InputController();
    ~InputController();
    
    // Запрет копирования
    InputController(const InputController&) = delete;
    InputController& operator=(const InputController&) = delete;
    
    // === ВНУТРЕННЕЕ СОСТОЯНИЕ ===
    
    // Клавиатура
    bool m_keyStates[256] = {false};
    bool m_keyPressedThisFrame[256] = {false};
    bool m_keyReleasedThisFrame[256] = {false};
    
    // Мышь
    glm::vec2 m_mousePosition = {0.0f, 0.0f};
    glm::vec2 m_mouseDelta = {0.0f, 0.0f};
    float m_mouseWheelDelta = 0.0f;
    bool m_mouseButtonStates[5] = {false};
    bool m_mousePressedThisFrame[5] = {false};
    bool m_mouseReleasedThisFrame[5] = {false};
    
    // Геймпады
    struct GamepadState {
        bool connected = false;
        bool buttons[14] = {false};
        float axes[6] = {0.0f};
    };
    GamepadState m_gamepads[4];
    
    // Модификаторы
    Modifiers m_currentModifiers;
    
    // Действия
    std::unordered_map<std::string, std::unique_ptr<InputAction>> m_actions;
    
    // Контексты
    std::vector<std::string> m_contextStack;
};

} // namespace ogle