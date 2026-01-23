// InputContext.h
#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "InputAction.h"
#include "InputAxis.h"

namespace ogle::input
{
    // Контекст ввода (например, "Gameplay", "UI", "Menu", "Vehicle")
    class InputContext
    {
    public:
        InputContext(const std::string& name);
        
        const std::string& GetName() const { return m_name; }
        bool IsActive() const { return m_isActive; }
        void SetActive(bool active) { m_isActive = active; }
        
        // Управление действиями
        InputAction* CreateAction(const std::string& name);
        InputAction* GetAction(const std::string& name);
        
        // Управление осями
        InputAxis* CreateAxis(const std::string& name);
        InputAxis* GetAxis(const std::string& name);
        
        // Обработка событий
        void ProcessEvent(const InputEvent& event, float deltaTime);
        
        // Обновление состояний
        void Update(float deltaTime);
        
        // Очистка
        void Clear();
        
    private:
        std::string m_name;
        bool m_isActive = true;
        
        std::unordered_map<std::string, std::unique_ptr<InputAction>> m_actions;
        std::unordered_map<std::string, std::unique_ptr<InputAxis>> m_axes;
    };
}