// InputAction.cpp
#include "InputAction.h"
#include "InputSystem.h"

namespace ogle::input
{
    InputAction::InputAction(const std::string& name) 
        : m_name(name) 
    {
    }
    
    void InputAction::AddKeyBinding(int keyCode, ActionTrigger trigger)
    {
        KeyBinding binding;
        binding.keyCode = keyCode;
        binding.trigger = trigger;
        m_keyBindings.push_back(binding);
    }
    
    void InputAction::AddMouseBinding(int button, ActionTrigger trigger)
    {
        KeyBinding binding;
        binding.keyCode = button;
        binding.trigger = trigger;
        m_mouseBindings.push_back(binding);
    }
    
    bool InputAction::ProcessEvent(const InputEvent& event, float deltaTime)
    {
        bool triggered = false;
        float value = 0.0f;
        
        // Обработка событий клавиатуры
        if (event.type == EventType::KeyPressed || 
            event.type == EventType::KeyReleased || 
            event.type == EventType::KeyHeld)
        {
            const auto& keyEvent = static_cast<const KeyEvent&>(event);
            
            for (const auto& binding : m_keyBindings)
            {
                if (binding.keyCode == keyEvent.keyCode)
                {
                    switch (binding.trigger)
                    {
                    case ActionTrigger::Pressed:
                        triggered = (event.type == EventType::KeyPressed);
                        break;
                    case ActionTrigger::Released:
                        triggered = (event.type == EventType::KeyReleased);
                        break;
                    case ActionTrigger::Held:
                        triggered = (event.type == EventType::KeyHeld);
                        value = 1.0f;
                        break;
                    case ActionTrigger::Toggle:
                        if (event.type == EventType::KeyPressed)
                        {
                            m_isPressed = !m_isPressed;
                            triggered = true;
                            value = m_isPressed ? 1.0f : 0.0f;
                        }
                        break;
                    }
                    
                    if (triggered) break;
                }
            }
        }
        
        // Обработка событий мыши
        else if (event.type == EventType::MouseButtonPressed || 
                 event.type == EventType::MouseButtonReleased)
        {
            const auto& mouseEvent = static_cast<const MouseButtonEvent&>(event);
            
            for (const auto& binding : m_mouseBindings)
            {
                if (binding.keyCode == mouseEvent.button)
                {
                    switch (binding.trigger)
                    {
                    case ActionTrigger::Pressed:
                        triggered = (event.type == EventType::MouseButtonPressed);
                        break;
                    case ActionTrigger::Released:
                        triggered = (event.type == EventType::MouseButtonReleased);
                        break;
                    case ActionTrigger::Held:
                        triggered = (event.type == EventType::MouseButtonPressed);
                        value = 1.0f;
                        break;
                    case ActionTrigger::Toggle:
                        if (event.type == EventType::MouseButtonPressed)
                        {
                            m_isPressed = !m_isPressed;
                            triggered = true;
                            value = m_isPressed ? 1.0f : 0.0f;
                        }
                        break;
                    }
                    
                    if (triggered) break;
                }
            }
        }
        
        if (triggered)
        {
            m_wasPressedThisFrame = (value > 0.0f);
            m_value = value;
            
            if (m_callback)
            {
                m_callback(event, value);
            }
        }
        
        return triggered;
    }
    
    void InputAction::Trigger(float value)
    {
        m_value = value;
        m_wasPressedThisFrame = true;
        
        // Создаем искусственное событие
        KeyEvent event;
        event.type = EventType::KeyPressed;
        
        if (m_callback)
        {
            m_callback(event, value);
        }
    }
}