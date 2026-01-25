// src/systems/GuiSystem.h
#pragma once

#define NOMINMAX
#include <algorithm>
#include "core/ISystem.h"
#include "ui/UIController.h"

struct ImGuiContext;

namespace ogle {

class GuiSystem final : public ISystem {
public:
    GuiSystem();
    ~GuiSystem() override;

    // === ISystem интерфейс ===
    const std::string& GetName() const override;
    bool Initialize() override;
    void Update(float deltaTime) override { /* пусто */ }
    void Render() override { /* пусто */ }
    void Shutdown() override;
    
    void OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;
    void OnResize(int width, int height) override;

    // === Основной метод ===
    void RenderUI();
    
    // === Установка окна ===
    void SetWindowHandle(HWND hwnd, HDC hdc);
    
    // === Флаги захвата ввода ===
    bool WantCaptureMouse() const;
    bool WantCaptureKeyboard() const;
    
    bool IsInitialized() const { return m_initialized; }

private:
    bool InitImGui();
    void ShutdownImGui();
    void DrawTestUI();

    // ImGui контекст
    ImGuiContext* m_imguiContext = nullptr;
    
    // Окно
    HWND m_hwnd = nullptr;
    HDC m_hdc = nullptr;
    
    // Флаги
    bool m_initialized = false;
    bool m_wantCaptureMouse = false;
    bool m_wantCaptureKeyboard = false;
};

} // namespace ogle