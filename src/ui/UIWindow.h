// src/ui/UIWindow.h
#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include "UIEnums.h"
#include "UIWidgets.h"

namespace ogle {

class UIWindow {
public:
    UIWindow(const std::string& id, const std::string& title);
    ~UIWindow();
    
    // Управление окном
    void Show();
    void Hide();
    void Toggle();
    bool IsVisible() const { return m_visible; }
    
    // Настройки
    void SetTitle(const std::string& title) { m_title = title; }
    void SetFlags(WindowFlags flags) { m_flags = flags; }
    void SetOpenPtr(bool* open) { m_open = open; }
    void SetPosition(const ImVec2& pos) { m_position = pos; m_hasPosition = true; }
    void SetSize(const ImVec2& size) { m_size = size; m_hasSize = true; }
    
    // Управление виджетами
    UIWidget* AddWidget(WidgetType type, const std::string& id);
    UIWidget* GetWidget(const std::string& id);
    void RemoveWidget(const std::string& id);
    void ClearWidgets();
    
    // Рендеринг
    void Begin();
    void End();
    void Render();
    
    // Быстрые методы добавления
    UIButton* AddButton(const std::string& id, const std::string& label, const ImVec2& size = {0, 0});
    UILabel* AddLabel(const std::string& id, const std::string& text);
    UICheckbox* AddCheckbox(const std::string& id, const std::string& label, bool* value = nullptr);
    UISliderFloat* AddSliderFloat(const std::string& id, const std::string& label, 
                                 float* value = nullptr, float min = 0.0f, float max = 1.0f);
    UIInputText* AddInputText(const std::string& id, const std::string& label, 
                             std::string* text = nullptr);
    UICombo* AddCombo(const std::string& id, const std::string& label, 
                     const std::vector<std::string>& items, int* currentIndex = nullptr);
    
private:
    std::string m_id;
    std::string m_title;
    bool m_visible = true;
    bool* m_open = nullptr;
    WindowFlags m_flags = WindowFlags::None;
    
    ImVec2 m_position = {0, 0};
    ImVec2 m_size = {0, 0};
    bool m_hasPosition = false;
    bool m_hasSize = false;
    
    std::vector<std::unique_ptr<UIWidget>> m_widgets;
};

} // namespace ogle