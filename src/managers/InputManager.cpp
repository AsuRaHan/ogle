#include "managers/InputManager.h"

#include "input/InputController.h"
#include "input/InputTypes.h"
#include "ui/IWindow.h"

#include <algorithm>
#include <glm/vec2.hpp>
#include <windowsx.h>

InputManager::InputManager() = default;

void InputManager::AttachToWindow(IWindow& window)
{
    window.AddMessageObserver([this](UINT msg, WPARAM wParam, LPARAM lParam) {
        HandleWindowMessage(msg, wParam, lParam);
    });
}

void InputManager::Update(float deltaTime)
{
    auto& input = ogle::InputController::Get();
    input.ResetFrameState();

    bool keyPressed[256] = { false };
    bool keyReleased[256] = { false };
    for (size_t i = 0; i < m_keyStates.size(); ++i) {
        keyPressed[i] = m_keyStates[i] && !m_previousKeyStates[i];
        keyReleased[i] = !m_keyStates[i] && m_previousKeyStates[i];
        m_previousKeyStates[i] = m_keyStates[i];
    }

    bool mousePressed[5] = { false };
    bool mouseReleased[5] = { false };
    for (size_t i = 0; i < m_mouseButtonStates.size(); ++i) {
        mousePressed[i] = m_mouseButtonStates[i] && !m_previousMouseButtonStates[i];
        mouseReleased[i] = !m_mouseButtonStates[i] && m_previousMouseButtonStates[i];
        m_previousMouseButtonStates[i] = m_mouseButtonStates[i];
    }

    const glm::vec2 mousePosition(
        static_cast<float>(m_mousePosition.x),
        static_cast<float>(m_mousePosition.y));
    const glm::vec2 mouseDelta(
        static_cast<float>(m_mousePosition.x - m_previousMousePosition.x),
        static_cast<float>(m_mousePosition.y - m_previousMousePosition.y));
    m_previousMousePosition = m_mousePosition;

    input.UpdateKeyboardState(m_keyStates.data(), keyPressed, keyReleased);
    input.UpdateMouseState(
        mousePosition,
        mouseDelta,
        m_mouseWheelDelta,
        m_mouseButtonStates.data(),
        mousePressed,
        mouseReleased);

    ogle::Modifiers modifiers;
    modifiers.ctrl = m_keyStates[VK_CONTROL] || m_keyStates[VK_LCONTROL] || m_keyStates[VK_RCONTROL];
    modifiers.shift = m_keyStates[VK_SHIFT] || m_keyStates[VK_LSHIFT] || m_keyStates[VK_RSHIFT];
    modifiers.alt = m_keyStates[VK_MENU] || m_keyStates[VK_LMENU] || m_keyStates[VK_RMENU];
    modifiers.win = m_keyStates[VK_LWIN] || m_keyStates[VK_RWIN];
    input.UpdateModifiers(modifiers);
    input.UpdateActions(deltaTime);

    m_mouseWheelDelta = 0.0f;
}

void InputManager::ResetAllStates()
{
    m_keyStates.fill(false);
    m_previousKeyStates.fill(false);
    m_mouseButtonStates.fill(false);
    m_previousMouseButtonStates.fill(false);
    m_mouseWheelDelta = 0.0f;
    m_previousMousePosition = m_mousePosition;
}

void InputManager::HandleWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KILLFOCUS:
    case WM_CAPTURECHANGED:
        ResetAllStates();
        break;

    case WM_ACTIVATEAPP:
        if (wParam == FALSE) {
            ResetAllStates();
        }
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (wParam < m_keyStates.size())
            m_keyStates[static_cast<size_t>(wParam)] = true;
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wParam < m_keyStates.size())
            m_keyStates[static_cast<size_t>(wParam)] = false;
        break;

    case WM_MOUSEMOVE:
        m_mousePosition.x = GET_X_LPARAM(lParam);
        m_mousePosition.y = GET_Y_LPARAM(lParam);
        break;

    case WM_MOUSEWHEEL:
        m_mouseWheelDelta += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP: {
        const int button = TranslateMouseButton(msg, wParam);
        if (button >= 0) {
            const bool isDown =
                msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN ||
                msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN;
            m_mouseButtonStates[static_cast<size_t>(button)] = isDown;
        }
        break;
    }

    default:
        break;
    }
}

int InputManager::TranslateMouseButton(UINT msg, WPARAM wParam)
{
    switch (msg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        return 0;

    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        return 1;

    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        return 2;

    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 3 : 4;

    default:
        return -1;
    }
}
