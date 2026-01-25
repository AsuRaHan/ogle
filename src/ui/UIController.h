// src/ui/UIController.h
#pragma once

#include <imgui.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include "core/EventSystem.h"
#include "log/Logger.h"

namespace ogle {

// Базовый класс UI элемента
class UIElement {
public:
    virtual ~UIElement() = default;
    
    std::string id;
    std::string label;
    bool visible = true;
    bool enabled = true;
    
    virtual void Draw() = 0;
    virtual void Update(float deltaTime) {}
    
    virtual void OnShow() {}
    virtual void OnHide() {}
};

// Конкретные элементы
class UIButton : public UIElement {
public:
    std::function<void()> onClick;
    ImVec2 size = {0, 0};
    
    void Draw() override {
        if (!visible) return;
        
        ImGui::PushID(id.c_str());
        
        if (!enabled) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }
        
        if (ImGui::Button(label.c_str(), size) && enabled && onClick) {
            onClick();
        }
        
        if (!enabled) {
            ImGui::PopStyleColor(3);
        }
        
        ImGui::PopID();
    }
};

class UILabel : public UIElement {
public:
    std::string text;
    
    void Draw() override {
        if (!visible) return;
        ImGui::Text("%s", text.c_str());
    }
};

// Контроллер UI
class UIController {
public:
    static UIController& Get();
    
    // === Управление окнами ===
    // Только регистрация окна (без вызова ImGui::Begin)
    void RegisterWindow(const std::string& windowId, 
                       const std::string& title, 
                       bool* open = nullptr,
                       ImGuiWindowFlags flags = 0);
    
    // Добавление элемента в окно
    void AddToWindow(const std::string& windowId, const std::string& elementId);
    
    // === Создание элементов ===
    UIButton* CreateButton(const std::string& id, 
                          const std::string& label,
                          const ImVec2& size = {0, 0});
    
    UILabel* CreateLabel(const std::string& id, 
                        const std::string& text);
    
    // === Event Button ===
    UIButton* CreateEventButton(const std::string& id,
                               const std::string& label,
                               const std::string& eventName,
                               const ImVec2& size = {0, 0});
    
    // === Управление элементами ===
    UIElement* GetElement(const std::string& id);
    template<typename T> T* GetElementAs(const std::string& id);
    
    void RemoveElement(const std::string& id);
    void ShowElement(const std::string& id);
    void HideElement(const std::string& id);
    
    // === Обновление и рендеринг ===
    // ЭТОТ МЕТОД ДОЛЖЕН ВЫЗЫВАТЬСЯ ТОЛЬКО ПОСЛЕ ImGui::NewFrame()
    void Render();
    
    // === EventSystem доступ ===
    EventSystem& GetEventSystem() { return m_eventSystem; }
    
    // === Статистика ===
    size_t GetElementCount() const { return m_elements.size(); }
    size_t GetWindowCount() const { return m_windows.size(); }

    const std::vector<std::string>* GetWindowElementIds(const std::string& windowId) const {
        auto it = m_windows.find(windowId);
        return it != m_windows.end() ? &it->second.elementIds : nullptr;
    }
private:
    UIController() = default;
    
    struct UIWindow {
        std::string id;
        std::string title;
        std::vector<std::string> elementIds;
        bool* open = nullptr;
        ImGuiWindowFlags flags = 0;
        bool visible = true;
    };
    
    std::unordered_map<std::string, std::unique_ptr<UIElement>> m_elements;
    std::unordered_map<std::string, UIWindow> m_windows;
    
    EventSystem m_eventSystem;
};

template<typename T>
T* UIController::GetElementAs(const std::string& id) {
    UIElement* element = GetElement(id);
    return element ? dynamic_cast<T*>(element) : nullptr;
}

} // namespace ogle