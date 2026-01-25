// src/ui/UIController.cpp
#include "UIController.h"
#include <algorithm>

namespace ogle {

UIController& UIController::Get() {
    static UIController instance;
    return instance;
}

// === Окна ===
void UIController::RegisterWindow(const std::string& windowId,
                                 const std::string& title,
                                 bool* open,
                                 ImGuiWindowFlags flags) {
    // Только регистрируем, не вызываем ImGui!
    m_windows[windowId] = {windowId, title, {}, open, flags, true};
    Logger::Debug("UI Window registered: " + windowId);
}

void UIController::AddToWindow(const std::string& windowId, const std::string& elementId) {
    auto windowIt = m_windows.find(windowId);
    if (windowIt != m_windows.end()) {
        auto& elementIds = windowIt->second.elementIds;
        
        // Проверяем, нет ли уже такого элемента
        if (std::find(elementIds.begin(), elementIds.end(), elementId) == elementIds.end()) {
            elementIds.push_back(elementId);
            Logger::Debug("Element " + elementId + " added to window " + windowId);
        }
    }
}

// === Создание элементов ===
UIButton* UIController::CreateButton(const std::string& id,
                                    const std::string& label,
                                    const ImVec2& size) {
    if (m_elements.find(id) != m_elements.end()) {
        Logger::Warning("UI Element already exists: " + id);
        return dynamic_cast<UIButton*>(m_elements[id].get());
    }
    
    auto button = std::make_unique<UIButton>();
    button->id = id;
    button->label = label;
    button->size = size;
    
    UIButton* ptr = button.get();
    m_elements[id] = std::move(button);
    
    Logger::Debug("UI Button created: " + id);
    return ptr;
}

UILabel* UIController::CreateLabel(const std::string& id,
                                  const std::string& text) {
    if (m_elements.find(id) != m_elements.end()) {
        Logger::Warning("UI Element already exists: " + id);
        return dynamic_cast<UILabel*>(m_elements[id].get());
    }
    
    auto label = std::make_unique<UILabel>();
    label->id = id;
    label->text = text;
    
    UILabel* ptr = label.get();
    m_elements[id] = std::move(label);
    
    Logger::Debug("UI Label created: " + id);
    return ptr;
}

UIButton* UIController::CreateEventButton(const std::string& id,
                                         const std::string& label,
                                         const std::string& eventName,
                                         const ImVec2& size) {
    UIButton* button = CreateButton(id, label, size);
    if (button) {
        button->onClick = [this, eventName]() {
            m_eventSystem.Emit(eventName);
        };
        Logger::Debug("Event button created: " + id + " -> " + eventName);
    }
    return button;
}

// === Управление элементами ===
UIElement* UIController::GetElement(const std::string& id) {
    auto it = m_elements.find(id);
    return it != m_elements.end() ? it->second.get() : nullptr;
}

void UIController::RemoveElement(const std::string& id) {
    // Удаляем из окон
    for (auto& window : m_windows) {
        auto& elements = window.second.elementIds;
        elements.erase(std::remove(elements.begin(), elements.end(), id),
                      elements.end());
    }
    
    // Удаляем элемент
    if (m_elements.erase(id) > 0) {
        Logger::Debug("UI Element removed: " + id);
    }
}

void UIController::ShowElement(const std::string& id) {
    UIElement* element = GetElement(id);
    if (element && !element->visible) {
        element->visible = true;
        element->OnShow();
        Logger::Debug("UI Element shown: " + id);
    }
}

void UIController::HideElement(const std::string& id) {
    UIElement* element = GetElement(id);
    if (element && element->visible) {
        element->visible = false;
        element->OnHide();
        Logger::Debug("UI Element hidden: " + id);
    }
}

// === Рендеринг ===
void UIController::Render() {
    // ВАЖНО: Этот метод должен вызываться ТОЛЬКО после ImGui::NewFrame()
    // и ДО ImGui::EndFrame()
    
    // Рендерим все видимые окна
    for (auto& windowPair : m_windows) {
        auto& window = windowPair.second;
        
        if (!window.visible) continue;
        
        // Начинаем окно ImGui
        ImGui::Begin(window.title.c_str(), window.open, window.flags);
        
        // Рендерим элементы этого окна
        for (const auto& elementId : window.elementIds) {
            auto it = m_elements.find(elementId);
            if (it != m_elements.end() && it->second->visible) {
                it->second->Draw();
            }
        }
        
        ImGui::End();
    }
}

} // namespace ogle