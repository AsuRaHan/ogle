// src/systems/InputSystem.cpp
#include "InputSystem.h"
#include "log/Logger.h"
#include "managers/InputManager.h"
#include "input/InputController.h"  // Добавляем
#include <windowsx.h>
#include <XInput.h>
#pragma comment(lib, "XInput.lib")

namespace ogle {

// Вспомогательные функции остаются
float InputSystem::NormalizeAxis(SHORT value) {
    return (value > 0) ? value / 32767.0f : value / 32768.0f;
}

void InputSystem::ApplyDeadzone(float& x, float& y, SHORT deadzone) {
    float magnitude = sqrtf(x * x + y * y);
    if (magnitude < deadzone / 32767.0f) {
        x = y = 0.0f;
    }
    else {
        float scale = (magnitude - deadzone / 32767.0f) / (1.0f - deadzone / 32767.0f);
        x = x / magnitude * scale;
        y = y / magnitude * scale;
    }
}

InputSystem::InputSystem() {
    memset(m_keyStates, 0, sizeof(m_keyStates));
    memset(m_keyPressedThisFrame, 0, sizeof(m_keyPressedThisFrame));
    memset(m_keyReleasedThisFrame, 0, sizeof(m_keyReleasedThisFrame));

    for (auto& gamepad : m_gamepads) {
        gamepad.Reset();
    }
}

InputSystem::~InputSystem() {
    SetMouseCapture(false);
}

void InputSystem::Shutdown() {
    // Cleanup
}

const std::string& InputSystem::GetName() const {
    static const std::string name = "InputSystem";
    return name;
}

bool InputSystem::Initialize() {
    Logger::Info("InputSystem initialized");
    return true;
}

// В методе InputSystem::Update():
void InputSystem::Update(float deltaTime) {
    UpdateGamepads();
    UpdateController();  // Обновляем контроллер

    // Обновляем действия в контроллере
    InputController::Get().UpdateActions(deltaTime);

    // Создаем std::array для InputManager
    std::array<bool, 256> keyStatesArray;
    for (int i = 0; i < 256; ++i) {
        keyStatesArray[i] = m_keyStates[i];
    }

    // Управление камерой через InputManager
    //InputManager::Get().Update(deltaTime,
    //    keyStatesArray,  // std::array
    //    m_mouseDelta,
    //    m_mouseWheelDelta,
    //    m_mouseButtonStates[1]);  // Правая кнопка мыши

    ResetFrameState();
}

void InputSystem::OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    // Обновляем модификаторы
    m_currentModifiers = Modifiers(GetKeyState(VK_CONTROL) | GetKeyState(VK_SHIFT) |
        GetKeyState(VK_MENU) | GetKeyState(VK_LWIN));

    switch (msg) {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        ProcessKeyEvent(msg, wParam, lParam);
        break;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP:
    case WM_XBUTTONDOWN: case WM_XBUTTONUP:
    case WM_MOUSEWHEEL:
        ProcessMouseEvent(msg, wParam, lParam);
        break;

    case WM_INPUT:
        ProcessRawInput(lParam);
        break;
    }
}

void InputSystem::ProcessKeyEvent(UINT msg, WPARAM wParam, LPARAM lParam) {
    int vk = static_cast<int>(wParam);
    bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
    bool repeat = (lParam & 0x40000000) != 0;

    if (pressed) {
        // Передаём нажатую клавишу в менеджер
        InputManager::Get().ProcessKey(vk);
    }

    if (pressed != m_keyStates[vk]) {
        m_keyStates[vk] = pressed;
        if (pressed) {
            m_keyPressedThisFrame[vk] = true;
        }
        else {
            m_keyReleasedThisFrame[vk] = true;
        }
    }
}

void InputSystem::ProcessMouseEvent(UINT msg, WPARAM wParam, LPARAM lParam) {
    glm::vec2 pos = {
        static_cast<float>(GET_X_LPARAM(lParam)),
        static_cast<float>(GET_Y_LPARAM(lParam))
    };

    glm::vec2 delta = pos - m_mousePosition;
    m_mousePosition = pos;

    if (!m_mouseCaptured) {
        m_mouseDelta += delta;
    }

    // Обработка кнопок мыши
    int button = -1;
    bool pressed = false;

    switch (msg) {
    case WM_LBUTTONDOWN: button = 0; pressed = true; break;
    case WM_LBUTTONUP:   button = 0; pressed = false; break;
    case WM_RBUTTONDOWN: button = 1; pressed = true; break;
    case WM_RBUTTONUP:   button = 1; pressed = false; break;
    case WM_MBUTTONDOWN: button = 2; pressed = true; break;
    case WM_MBUTTONUP:   button = 2; pressed = false; break;
    case WM_XBUTTONDOWN: {
        int xbutton = GET_XBUTTON_WPARAM(wParam);
        button = (xbutton == XBUTTON1) ? 3 : 4;
        pressed = true;
        break;
    }
    case WM_XBUTTONUP: {
        int xbutton = GET_XBUTTON_WPARAM(wParam);
        button = (xbutton == XBUTTON1) ? 3 : 4;
        pressed = false;
        break;
    }
    case WM_MOUSEWHEEL: {
        m_mouseWheelDelta += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
        break;
    }
    }

    if (button != -1) {
        if (pressed != m_mouseButtonStates[button]) {
            m_mouseButtonStates[button] = pressed;
            if (pressed) {
                m_mousePressedThisFrame[button] = true;
            }
            else {
                m_mouseReleasedThisFrame[button] = true;
            }
        }
    }
}

void InputSystem::ProcessRawInput(LPARAM lParam) {
    UINT size = sizeof(RAWINPUT);
    static BYTE buffer[sizeof(RAWINPUT)];

    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));
    RAWINPUT* raw = (RAWINPUT*)buffer;

    if (raw->header.dwType == RIM_TYPEMOUSE) {
        if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE) {
            m_mouseDelta.x += static_cast<float>(raw->data.mouse.lLastX);
            m_mouseDelta.y += static_cast<float>(raw->data.mouse.lLastY);
        }
    }
}

void InputSystem::UpdateGamepads() {
    for (int i = 0; i < 4; ++i) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        DWORD result = XInputGetState(i, &state);
        m_gamepads[i].connected = (result == ERROR_SUCCESS);

        if (m_gamepads[i].connected) {
            // Кнопки
            WORD buttons = state.Gamepad.wButtons;
            m_gamepads[i].buttons[0] = (buttons & XINPUT_GAMEPAD_A) != 0;
            m_gamepads[i].buttons[1] = (buttons & XINPUT_GAMEPAD_B) != 0;
            m_gamepads[i].buttons[2] = (buttons & XINPUT_GAMEPAD_X) != 0;
            m_gamepads[i].buttons[3] = (buttons & XINPUT_GAMEPAD_Y) != 0;
            m_gamepads[i].buttons[4] = (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
            m_gamepads[i].buttons[5] = (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
            m_gamepads[i].buttons[6] = (buttons & XINPUT_GAMEPAD_BACK) != 0;
            m_gamepads[i].buttons[7] = (buttons & XINPUT_GAMEPAD_START) != 0;
            m_gamepads[i].buttons[8] = (buttons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
            m_gamepads[i].buttons[9] = (buttons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
            m_gamepads[i].buttons[10] = (buttons & XINPUT_GAMEPAD_DPAD_UP) != 0;
            m_gamepads[i].buttons[11] = (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
            m_gamepads[i].buttons[12] = (buttons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
            m_gamepads[i].buttons[13] = (buttons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;

            // Стики
            m_gamepads[i].axes[0] = NormalizeAxis(state.Gamepad.sThumbLX);
            m_gamepads[i].axes[1] = NormalizeAxis(state.Gamepad.sThumbLY);
            m_gamepads[i].axes[2] = NormalizeAxis(state.Gamepad.sThumbRX);
            m_gamepads[i].axes[3] = NormalizeAxis(state.Gamepad.sThumbRY);

            // Триггеры
            m_gamepads[i].axes[4] = state.Gamepad.bLeftTrigger / 255.0f;
            m_gamepads[i].axes[5] = state.Gamepad.bRightTrigger / 255.0f;

            // Deadzone
            ApplyDeadzone(m_gamepads[i].axes[0], m_gamepads[i].axes[1], XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            ApplyDeadzone(m_gamepads[i].axes[2], m_gamepads[i].axes[3], XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        }
    }
}

// НОВЫЙ МЕТОД: обновление контроллера
void InputSystem::UpdateController() {
    auto& controller = InputController::Get();
    
    // Обновляем состояние клавиатуры
    controller.UpdateKeyboardState(
        m_keyStates,
        m_keyPressedThisFrame,
        m_keyReleasedThisFrame
    );
    
    // Обновляем состояние мыши
    controller.UpdateMouseState(
        m_mousePosition,
        m_mouseDelta,
        m_mouseWheelDelta,
        m_mouseButtonStates,
        m_mousePressedThisFrame,
        m_mouseReleasedThisFrame
    );
    
    // Обновляем состояние геймпадов
    for (int i = 0; i < 4; ++i) {
        controller.UpdateGamepadState(
            i,
            m_gamepads[i].connected,
            m_gamepads[i].buttons,
            m_gamepads[i].axes
        );
    }
    
    // Обновляем модификаторы
    controller.UpdateModifiers(m_currentModifiers);
    
    // Обновляем действия (передаем deltaTime из Update метода)
    // controller.UpdateActions(deltaTime); // Будет вызываться в Update
}

void InputSystem::ResetFrameState() {
    memset(m_keyPressedThisFrame, 0, sizeof(m_keyPressedThisFrame));
    memset(m_keyReleasedThisFrame, 0, sizeof(m_keyReleasedThisFrame));

    memset(m_mousePressedThisFrame, 0, sizeof(m_mousePressedThisFrame));
    memset(m_mouseReleasedThisFrame, 0, sizeof(m_mouseReleasedThisFrame));

    m_mouseDelta = { 0.0f, 0.0f };
    m_mouseWheelDelta = 0.0f;
}

// === Захват мыши (оставляем как внутреннюю функцию) ===

void InputSystem::SetMouseCapture(bool capture) {
    if (capture == m_mouseCaptured) return;

    m_mouseCaptured = capture;

    if (capture) {
        GetClientRect(m_hwnd, &m_clipRect);
        ClientToScreen(m_hwnd, reinterpret_cast<POINT*>(&m_clipRect.left));
        ClientToScreen(m_hwnd, reinterpret_cast<POINT*>(&m_clipRect.right));

        RECT rc;
        GetClientRect(m_hwnd, &rc);
        POINT center = { rc.right / 2, rc.bottom / 2 };
        ClientToScreen(m_hwnd, &center);
        SetCursorPos(center.x, center.y);
        m_lastMousePos = center;

        ClipCursor(&m_clipRect);
        ShowCursor(FALSE);
    }
    else {
        ClipCursor(nullptr);
        ShowCursor(TRUE);
        m_mouseDelta = { 0.0f, 0.0f };
    }
}

void InputSystem::SetWindowHandle(HWND hwnd) {
    m_hwnd = hwnd;

    // Опционально: настроить RAW input, если нужно
    if (m_hwnd) {
        // Регистрация для RAW input мыши
        RAWINPUTDEVICE rid[1];
        rid[0].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
        rid[0].usUsage = 0x02;     // HID_USAGE_GENERIC_MOUSE
        rid[0].dwFlags = 0;
        rid[0].hwndTarget = m_hwnd;
        RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
    }

    Logger::Debug("InputSystem window handle set: " + std::to_string(reinterpret_cast<uintptr_t>(hwnd)));
}

} // namespace ogle