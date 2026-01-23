// InputContext.cpp
#include "InputContext.h"

namespace ogle::input
{
    InputContext::InputContext(const std::string& name) 
        : m_name(name) 
    {
    }
    
    InputAction* InputContext::CreateAction(const std::string& name)
    {
        if (m_actions.find(name) != m_actions.end())
        {
            return m_actions[name].get();
        }
        
        auto action = std::make_unique<InputAction>(name);
        auto* ptr = action.get();
        m_actions[name] = std::move(action);
        return ptr;
    }
    
    InputAction* InputContext::GetAction(const std::string& name)
    {
        auto it = m_actions.find(name);
        if (it != m_actions.end())
        {
            return it->second.get();
        }
        return nullptr;
    }
    
    InputAxis* InputContext::CreateAxis(const std::string& name)
    {
        if (m_axes.find(name) != m_axes.end())
        {
            return m_axes[name].get();
        }
        
        auto axis = std::make_unique<InputAxis>(name);
        auto* ptr = axis.get();
        m_axes[name] = std::move(axis);
        return ptr;
    }
    
    InputAxis* InputContext::GetAxis(const std::string& name)
    {
        auto it = m_axes.find(name);
        if (it != m_axes.end())
        {
            return it->second.get();
        }
        return nullptr;
    }
    
    void InputContext::ProcessEvent(const InputEvent& event, float deltaTime)
    {
        // Обработка действий
        for (auto& [name, action] : m_actions)
        {
            if (action)
            {
                action->ProcessEvent(event, deltaTime);
            }
        }
        
        // События движения мыши могут обновлять оси
        if (event.type == EventType::MouseMoved)
        {
            Update(deltaTime);
        }
    }
    
    void InputContext::Update(float deltaTime)
    {
        // Обновляем оси
        for (auto& [name, axis] : m_axes)
        {
            if (axis)
            {
                axis->Update(deltaTime);
            }
        }
    }
    
    void InputContext::Clear()
    {
        m_actions.clear();
        m_axes.clear();
    }
}