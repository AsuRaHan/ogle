// src/ui/UIStyle.h
#pragma once

#include <imgui.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace ogle {

class UIStyle {
public:
    static UIStyle& Get();
    
    // Цвета
    void SetColor(ImGuiCol idx, const ImVec4& color);
    ImVec4 GetColor(ImGuiCol idx) const;
    
    // Размеры и отступы
    void SetStyleVar(ImGuiStyleVar idx, float value);
    void SetStyleVar(ImGuiStyleVar idx, const ImVec2& value);
    
    // Шрифты
    void SetFontScale(float scale);
    float GetFontScale() const { return m_fontScale; }
    
    // Темы
    void ApplyDarkTheme();
    void ApplyLightTheme();
    void ApplyClassicTheme();
    void ApplyCustomTheme(const std::string& name);
    
    // Сохранение/загрузка
    void SaveToFile(const std::string& filename);
    bool LoadFromFile(const std::string& filename);
    
private:
    UIStyle();
    
    float m_fontScale = 1.0f;
    std::unordered_map<std::string, std::string> m_customThemes;
};

} // namespace ogle