// InputAction.h
#pragma once
#include <string>
#include <vector>
#include <functional>
#include "InputEvents.h"

namespace ogle::input
{
    // Режимы триггера действия
    enum class ActionTrigger
    {
        Pressed,    // Только при нажатии
        Released,   // Только при отпускании
        Held,       // Пока удерживается
        Toggle      // Переключатель
    };

    // Класс действия (например, "Jump", "Fire", "Interact")
    class InputAction
    {
    public:
        using Callback = std::function<void(const InputEvent&, float)>;
        
        InputAction(const std::string& name);
        
        const std::string& GetName() const { return m_name; }
        
        // Привязка клавиш/кнопок
        void AddKeyBinding(int keyCode, ActionTrigger trigger = ActionTrigger::Pressed);
        // Overload for char literals - normalize to uppercase/VK
        void AddKeyBinding(char keyChar, ActionTrigger trigger = ActionTrigger::Pressed)
        {
            int kc = static_cast<int>(keyChar);
            if (kc >= 'a' && kc <= 'z') kc = kc - ('a' - 'A');
            AddKeyBinding(kc, trigger);
        }
        void AddMouseBinding(int button, ActionTrigger trigger = ActionTrigger::Pressed);
        void AddGamepadBinding(int button, ActionTrigger trigger = ActionTrigger::Pressed);
        
        // Установка коллбека
        void SetCallback(const Callback& callback) { m_callback = callback; }
        
        // Обработка события
        bool ProcessEvent(const InputEvent& event, float deltaTime);
        
        // Ручной триггер (для скриптов/UI)
        void Trigger(float value = 1.0f);
        
        // Состояние
        bool IsPressed() const { return m_isPressed; }
        bool WasPressedThisFrame() const { return m_wasPressedThisFrame; }
        float GetValue() const { return m_value; }
        
    private:
        std::string m_name;
        Callback m_callback;
        
        struct KeyBinding
        {
            int keyCode;
            ActionTrigger trigger;
        };
        
        std::vector<KeyBinding> m_keyBindings;
        std::vector<KeyBinding> m_mouseBindings;
        std::vector<KeyBinding> m_gamepadBindings;
        
        bool m_isPressed = false;
        bool m_wasPressedThisFrame = false;
        bool m_wasReleasedThisFrame = false;
        float m_value = 0.0f;
        float m_pressTime = 0.0f;
    };
}