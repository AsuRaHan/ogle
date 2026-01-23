// InputEvents.h
#pragma once
#include <glm/glm.hpp>
#include <string>
#include <functional>

namespace ogle::input
{
    // Типы событий
    enum class EventType
    {
        KeyPressed,
        KeyReleased,
        KeyHeld,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled,
        GamepadButtonPressed,
        GamepadButtonReleased,
        GamepadAxisMoved,
        TouchBegan,
        TouchMoved,
        TouchEnded
    };

    // Базовое событие ввода
    struct InputEvent
    {
        EventType type;
        float timestamp;
        bool consumed = false;
        
        virtual ~InputEvent() = default;
    };

    // Событие клавиши
    struct KeyEvent : InputEvent
    {
        int keyCode;
        bool shift = false;
        bool ctrl = false;
        bool alt = false;
        bool super = false;  // Windows/Command
    };

    // Событие мыши
    struct MouseButtonEvent : InputEvent
    {
        int button;
        glm::vec2 position;
    };

    struct MouseMoveEvent : InputEvent
    {
        glm::vec2 position;
        glm::vec2 delta;
    };

    struct MouseScrollEvent : InputEvent
    {
        glm::vec2 offset;  // x - горизонтальная прокрутка, y - вертикальная
    };

    // Обработчик событий (коллбек)
    using EventHandler = std::function<void(const InputEvent&)>;
}