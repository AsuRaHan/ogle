// InputAxis.h
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

namespace ogle::input
{
    // Ось ввода (например, "MoveHorizontal", "LookVertical")
    class InputAxis
    {
    public:
        using Callback = std::function<void(const glm::vec2&, float)>;
        
        InputAxis(const std::string& name);
        
        const std::string& GetName() const { return m_name; }
        
        // Привязка клавиш для оси (например, A/D для горизонтали)
        void AddKeyBinding(int negativeKey, int positiveKey);
        
        // Привязка мыши/геймпада
        void AddMouseAxisBinding(bool isXAxis = true, float sensitivity = 1.0f);
        void AddGamepadAxisBinding(int axisId, float sensitivity = 1.0f);
        
        // Установка мертвой зоны (для геймпадов)
        void SetDeadZone(float zone) { m_deadZone = zone; }
        
        // Установка коллбека
        void SetCallback(const Callback& callback) { m_callback = callback; }
        
        // Обновление значения
        void Update(float deltaTime);
        
        // Ручное управление
        void SetValue(const glm::vec2& value);
        
        // Получение значения
        const glm::vec2& GetValue() const { return m_value; }
        const glm::vec2& GetRawValue() const { return m_rawValue; }
        
    private:
        std::string m_name;
        Callback m_callback;
        
        struct KeyAxisBinding
        {
            int negativeKey;
            int positiveKey;
            float value;
        };
        
        struct MouseAxisBinding
        {
            bool isXAxis;
            float sensitivity;
        };
        
        std::vector<KeyAxisBinding> m_keyBindings;
        std::vector<MouseAxisBinding> m_mouseBindings;
        
        glm::vec2 m_value{0.0f};
        glm::vec2 m_rawValue{0.0f};
        float m_deadZone = 0.1f;
        float m_smoothTime = 0.1f;
        glm::vec2 m_velocity{0.0f};
    };
}