// InputAxis.cpp
#include "InputAxis.h"
#include "InputSystem.h"

namespace ogle::input
{
    InputAxis::InputAxis(const std::string& name) 
        : m_name(name) 
    {
    }
    
    void InputAxis::AddKeyBinding(int negativeKey, int positiveKey)
    {
        KeyAxisBinding binding;
        binding.negativeKey = negativeKey;
        binding.positiveKey = positiveKey;
        binding.value = 0.0f;
        m_keyBindings.push_back(binding);
    }
    
    void InputAxis::AddMouseAxisBinding(bool isXAxis, float sensitivity)
    {
        MouseAxisBinding binding;
        binding.isXAxis = isXAxis;
        binding.sensitivity = sensitivity;
        m_mouseBindings.push_back(binding);
    }
    
    void InputAxis::SetValue(const glm::vec2& value)
    {
        m_value = value;
        m_rawValue = value;
    }
    
    void InputAxis::Update(float deltaTime)
    {
        auto& inputSystem = InputSystem::GetInstance();
        glm::vec2 rawValue(0.0f);
        
        // Обработка клавишных биндингов
        for (auto& binding : m_keyBindings)
        {
            bool negative = inputSystem.IsKeyDown(binding.negativeKey);
            bool positive = inputSystem.IsKeyDown(binding.positiveKey);
            
            if (negative && !positive) 
                rawValue.x -= 1.0f;
            if (positive && !negative) 
                rawValue.x += 1.0f;
        }
        
        // Обработка мышиных биндингов
        glm::vec2 mouseDelta = inputSystem.GetMouseDelta();
        for (auto& binding : m_mouseBindings)
        {
            if (binding.isXAxis)
            {
                rawValue.x += mouseDelta.x * binding.sensitivity;
            }
            else
            {
                rawValue.y += mouseDelta.y * binding.sensitivity;
            }
        }
        
        // Применяем мертвую зону
        if (glm::length(rawValue) < m_deadZone)
        {
            rawValue = glm::vec2(0.0f);
        }
        
        // Нормализация для клавишных осей
        if (glm::length(rawValue) > 1.0f)
        {
            rawValue = glm::normalize(rawValue);
        }
        
        m_rawValue = rawValue;
        
        // Сглаживание (линейная интерполяция)
        if (m_smoothTime > 0.0f)
        {
            float t = deltaTime / m_smoothTime;
            t = glm::clamp(t, 0.0f, 1.0f);
            m_value = glm::mix(m_value, rawValue, t);
        }
        else
        {
            m_value = rawValue;
        }
        
        // Вызов коллбэка, если есть изменение
        if (m_callback && (m_value.x != 0.0f || m_value.y != 0.0f))
        {
            m_callback(m_value, deltaTime);
        }
    }
}