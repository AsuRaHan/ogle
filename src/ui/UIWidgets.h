// src/ui/UIWidgets.h
#pragma once

#include <imgui.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include "UIEnums.h"

namespace ogle {

// Базовый класс виджета
class UIWidget {
public:
    UIWidget(WidgetType type, const std::string& id);
    virtual ~UIWidget() = default;
    
    WidgetType GetType() const { return m_type; }
    const std::string& GetID() const { return m_id; }
    
    virtual void Draw() = 0;
    virtual void Update(float deltaTime) {}
    
    // Свойства
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }
    
protected:
    WidgetType m_type;
    std::string m_id;
    bool m_visible = true;
    bool m_enabled = true;
};

// Кнопка
class UIButton : public UIWidget {
public:
    UIButton(const std::string& id);
    
    void Draw() override;
    
    // Свойства
    void SetLabel(const std::string& label) { m_label = label; }
    void SetSize(const ImVec2& size) { m_size = size; }
    void SetOnClick(std::function<void()> callback) { m_onClick = callback; }
    
private:
    std::string m_label;
    ImVec2 m_size = {0, 0};
    std::function<void()> m_onClick;
};

// Текст
class UILabel : public UIWidget {
public:
    UILabel(const std::string& id);
    
    void Draw() override;
    
    void SetText(const std::string& text) { m_text = text; }
    void SetColor(const ImVec4& color) { m_color = color; }
    
private:
    std::string m_text;
    ImVec4 m_color = {1, 1, 1, 1};
};

// Чекбокс
class UICheckbox : public UIWidget {
public:
    UICheckbox(const std::string& id);
    
    void Draw() override;
    
    void SetLabel(const std::string& label) { m_label = label; }
    void SetValue(bool* value) { m_value = value; }
    void SetOnChange(std::function<void(bool)> callback) { m_onChange = callback; }
    
private:
    std::string m_label;
    bool* m_value = nullptr;
    bool m_localValue = false;
    std::function<void(bool)> m_onChange;
};

// Слайдер float
class UISliderFloat : public UIWidget {
public:
    UISliderFloat(const std::string& id);
    
    void Draw() override;
    
    void SetLabel(const std::string& label) { m_label = label; }
    void SetValue(float* value) { m_value = value; }
    void SetRange(float min, float max) { m_min = min; m_max = max; }
    void SetFormat(const std::string& format) { m_format = format; }
    void SetOnChange(std::function<void(float)> callback) { m_onChange = callback; }
    
private:
    std::string m_label;
    float* m_value = nullptr;
    float m_localValue = 0.0f;
    float m_min = 0.0f;
    float m_max = 1.0f;
    std::string m_format = "%.3f";
    std::function<void(float)> m_onChange;
};

// Поле ввода текста
class UIInputText : public UIWidget {
public:
    UIInputText(const std::string& id);
    
    void Draw() override;
    
    void SetLabel(const std::string& label) { m_label = label; }
    void SetText(std::string* text) { m_text = text; }
    void SetHint(const std::string& hint) { m_hint = hint; }
    void SetMaxLength(size_t max) { m_maxLength = max; }
    void SetOnChange(std::function<void(const std::string&)> callback) { m_onChange = callback; }
    
private:
    std::string m_label;
    std::string* m_text = nullptr;
    std::string m_localText;
    std::string m_hint;
    size_t m_maxLength = 256;
    std::function<void(const std::string&)> m_onChange;
};

// Выпадающий список
class UICombo : public UIWidget {
public:
    UICombo(const std::string& id);
    
    void Draw() override;
    
    void SetLabel(const std::string& label) { m_label = label; }
    void SetItems(const std::vector<std::string>& items) { m_items = items; }
    void SetCurrentIndex(int* index) { m_currentIndex = index; }
    void SetOnChange(std::function<void(int)> callback) { m_onChange = callback; }
    
private:
    std::string m_label;
    std::vector<std::string> m_items;
    int* m_currentIndex = nullptr;
    int m_localIndex = 0;
    std::function<void(int)> m_onChange;
};

// Фабрика виджетов
class UIWidgetFactory {
public:
    static std::unique_ptr<UIWidget> Create(WidgetType type, const std::string& id);
};

} // namespace ogle