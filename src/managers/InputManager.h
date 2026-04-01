#pragma once

#include <array>

#include <windows.h>

class IWindow;

class InputManager
{
public:
    InputManager();

    void AttachToWindow(IWindow& window);
    void Update(float deltaTime);

private:
    void HandleWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void ResetAllStates();
    static int TranslateMouseButton(UINT msg, WPARAM wParam);

    std::array<bool, 256> m_keyStates{};
    std::array<bool, 256> m_previousKeyStates{};
    std::array<bool, 5> m_mouseButtonStates{};
    std::array<bool, 5> m_previousMouseButtonStates{};
    POINT m_mousePosition{ 0, 0 };
    POINT m_previousMousePosition{ 0, 0 };
    float m_mouseWheelDelta = 0.0f;
};
