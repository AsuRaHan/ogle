// src/input/InputController.cpp
#include "input/InputController.h"

namespace ogle {

InputController& InputController::Get() {
    static InputController instance;
    return instance;
}

InputController::InputController() {
    Logger::Info("InputController initialized");
}

InputController::~InputController() {
    Logger::Info("InputController shutdown");
}

// === РЕАЛИЗАЦИЯ КЛАВИАТУРЫ ===

bool InputController::IsKeyDown(int vk) const {
    if (vk < 0 || vk >= 256) return false;
    return m_keyStates[vk];
}

bool InputController::IsKeyPressed(int vk) const {
    if (vk < 0 || vk >= 256) return false;
    return m_keyPressedThisFrame[vk];
}

bool InputController::IsKeyReleased(int vk) const {
    if (vk < 0 || vk >= 256) return false;
    return m_keyReleasedThisFrame[vk];
}

// === РЕАЛИЗАЦИЯ МЫШИ ===

glm::vec2 InputController::GetMousePosition() const {
    return m_mousePosition;
}

glm::vec2 InputController::GetMouseDelta() const {
    return m_mouseDelta;
}

float InputController::GetMouseWheelDelta() const {
    return m_mouseWheelDelta;
}

bool InputController::IsMouseButtonDown(int button) const {
    if (button < 0 || button >= 5) return false;
    return m_mouseButtonStates[button];
}

bool InputController::IsMouseButtonPressed(int button) const {
    if (button < 0 || button >= 5) return false;
    return m_mousePressedThisFrame[button];
}

bool InputController::IsMouseButtonReleased(int button) const {
    if (button < 0 || button >= 5) return false;
    return m_mouseReleasedThisFrame[button];
}

// === РЕАЛИЗАЦИЯ ГЕЙМПАДОВ ===

bool InputController::IsGamepadConnected(int player) const {
    if (player < 0 || player >= 4) return false;
    return m_gamepads[player].connected;
}

bool InputController::IsGamepadButtonDown(int player, int button) const {
    if (player < 0 || player >= 4) return false;
    if (button < 0 || button >= 14) return false;
    if (!m_gamepads[player].connected) return false;
    return m_gamepads[player].buttons[button];
}

float InputController::GetGamepadAxis(int player, int axis) const {
    if (player < 0 || player >= 4) return 0.0f;
    if (axis < 0 || axis >= 6) return 0.0f;
    if (!m_gamepads[player].connected) return 0.0f;
    return m_gamepads[player].axes[axis];
}

// === INPUT ACTIONS ===

InputAction* InputController::CreateAction(const std::string& name, ActionType type) {
    // Проверяем, есть ли уже действие с таким именем
    auto it = m_actions.find(name);
    if (it != m_actions.end()) {
        Logger::Warning("InputAction already exists: " + name);
        return it->second.get();
    }
    
    // Создаем новое действие
    auto action = std::make_unique<InputAction>(name, type);
    InputAction* ptr = action.get();
    m_actions[name] = std::move(action);
    
    Logger::Info("InputAction created: " + name);
    return ptr;
}

InputAction* InputController::GetAction(const std::string& name) {
    auto it = m_actions.find(name);
    if (it != m_actions.end()) {
        return it->second.get();
    }
    return nullptr;
}

void InputController::RemoveAction(const std::string& name) {
    auto it = m_actions.find(name);
    if (it != m_actions.end()) {
        m_actions.erase(it);
        Logger::Info("InputAction removed: " + name);
    }
}

bool InputController::HasAction(const std::string& name) const {
    return m_actions.find(name) != m_actions.end();
}

// === КОНТЕКСТЫ ===

void InputController::PushContext(const std::string& context) {
    m_contextStack.push_back(context);
    Logger::Debug("Input context pushed: " + context);
}

void InputController::PopContext() {
    if (!m_contextStack.empty()) {
        std::string context = m_contextStack.back();
        m_contextStack.pop_back();
        Logger::Debug("Input context popped: " + context);
    }
}

const std::string& InputController::GetCurrentContext() const {
    static const std::string empty = "";
    return m_contextStack.empty() ? empty : m_contextStack.back();
}

// === МОДИФИКАТОРЫ ===

const Modifiers& InputController::GetCurrentModifiers() const {
    return m_currentModifiers;
}

// === МЕТОДЫ ОБНОВЛЕНИЯ (для InputSystem) ===

void InputController::UpdateKeyboardState(
    const bool keyStates[256],
    const bool keyPressed[256],
    const bool keyReleased[256]) {
    
    for (int i = 0; i < 256; ++i) {
        m_keyStates[i] = keyStates[i];
        m_keyPressedThisFrame[i] = keyPressed[i];
        m_keyReleasedThisFrame[i] = keyReleased[i];
    }
}

void InputController::UpdateMouseState(
    const glm::vec2& position,
    const glm::vec2& delta,
    float wheelDelta,
    const bool buttonStates[5],
    const bool buttonPressed[5],
    const bool buttonReleased[5]) {
    
    m_mousePosition = position;
    m_mouseDelta = delta;
    m_mouseWheelDelta = wheelDelta;
    
    for (int i = 0; i < 5; ++i) {
        m_mouseButtonStates[i] = buttonStates[i];
        m_mousePressedThisFrame[i] = buttonPressed[i];
        m_mouseReleasedThisFrame[i] = buttonReleased[i];
    }
}

void InputController::UpdateGamepadState(
    int player,
    bool connected,
    const bool buttons[14],
    const float axes[6]) {
    
    if (player < 0 || player >= 4) return;
    
    m_gamepads[player].connected = connected;
    
    if (connected) {
        for (int i = 0; i < 14; ++i) {
            m_gamepads[player].buttons[i] = buttons[i];
        }
        
        for (int i = 0; i < 6; ++i) {
            m_gamepads[player].axes[i] = axes[i];
        }
    } else {
        // Сбрасываем состояние если геймпад отключен
        for (int i = 0; i < 14; ++i) {
            m_gamepads[player].buttons[i] = false;
        }
        
        for (int i = 0; i < 6; ++i) {
            m_gamepads[player].axes[i] = 0.0f;
        }
    }
}

void InputController::UpdateModifiers(const Modifiers& modifiers) {
    m_currentModifiers = modifiers;
}

void InputController::UpdateActions(float deltaTime) {
    // Обновляем модификаторы (если еще не обновлены)
    // m_currentModifiers уже должен быть обновлен InputSystem

    // Обновляем все действия
    for (auto& pair : m_actions) {
        pair.second->Update(deltaTime);
    }
}

void InputController::ResetFrameState() {
    // Сбрасываем временные состояния (нажато/отпущено в этом кадре)
    for (int i = 0; i < 256; ++i) {
        m_keyPressedThisFrame[i] = false;
        m_keyReleasedThisFrame[i] = false;
    }
    
    for (int i = 0; i < 5; ++i) {
        m_mousePressedThisFrame[i] = false;
        m_mouseReleasedThisFrame[i] = false;
    }
    
    // Сбрасываем дельту мыши и колесико
    m_mouseDelta = {0.0f, 0.0f};
    m_mouseWheelDelta = 0.0f;
    
    // Сбрасываем состояние действий
    for (auto& pair : m_actions) {
        pair.second->ResetFrameState();
    }
}

} // namespace ogle