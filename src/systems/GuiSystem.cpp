// src/systems/GuiSystem.cpp
#include "GuiSystem.h"
#include "log/Logger.h"

// ImGui
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl3.h"

// Win32
#include <windows.h>
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ogle {

GuiSystem::GuiSystem() {
    Logger::Info("GuiSystem created");
}

GuiSystem::~GuiSystem() {
    Shutdown();
}

const std::string& GuiSystem::GetName() const {
    static const std::string name = "GuiSystem";
    return name;
}

void GuiSystem::SetWindowHandle(HWND hwnd, HDC hdc) {
    if (m_hwnd || m_hdc) {
        Logger::Warning("GuiSystem window handle already set");
        return;
    }

    m_hwnd = hwnd;
    m_hdc = hdc;

    Logger::Debug("GuiSystem window handle set");
}

bool GuiSystem::Initialize() {
    Logger::Info("GuiSystem initializing...");

    if (!m_hwnd || !m_hdc) {
        Logger::Error("GuiSystem: No HWND/HDC provided. Call SetWindowHandle() first!");
        return false;
    }

    return InitImGui();
}

bool GuiSystem::InitImGui() {
    if (m_initialized) return true;

    // 1. Создаем контекст
    IMGUI_CHECKVERSION();
    m_imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_imguiContext);

    // 2. Настройка
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // 3. Стиль (ВАЖНО: WindowMinSize до StyleColorsDark)
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMinSize = ImVec2(100.0f, 100.0f);
    ImGui::StyleColorsDark();

    Logger::Info("ImGui context created");

    // 4. Бэкенды
    if (!ImGui_ImplWin32_Init(m_hwnd)) {
        Logger::Error("ImGui_ImplWin32_Init failed");
        ImGui::DestroyContext(m_imguiContext);
        m_imguiContext = nullptr;
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 460")) {
        Logger::Error("ImGui_ImplOpenGL3_Init failed");
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(m_imguiContext);
        m_imguiContext = nullptr;
        return false;
    }

    m_initialized = true;
    Logger::Info("ImGui initialized successfully");
    
    return true;
}

void GuiSystem::RenderUI() {
    if (!m_initialized) return;

    ImGui::SetCurrentContext(m_imguiContext);

    // Начинаем новый фрейм
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    UIController::Get().Render();

    // Рисуем UI
    DrawTestUI();

    // Завершаем фрейм
    ImGui::EndFrame();

    // Обновляем флаги захвата ввода
    ImGuiIO& io = ImGui::GetIO();
    m_wantCaptureMouse = io.WantCaptureMouse;
    m_wantCaptureKeyboard = io.WantCaptureKeyboard;

    // Рендерим
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Platform windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        // Получаем главный viewport
        ImGuiViewport* mainViewport = ImGui::GetMainViewport();

        // Ограничиваем все дочерние viewports размерами главного
        for (int i = 1; i < ImGui::GetPlatformIO().Viewports.Size; i++) {
            ImGuiViewport* viewport = ImGui::GetPlatformIO().Viewports[i];

            // Ограничиваем позицию и размер с помощью std::max/min
            viewport->Pos.x = std::max(viewport->Pos.x, mainViewport->Pos.x);
            viewport->Pos.y = std::max(viewport->Pos.y, mainViewport->Pos.y);

            viewport->Size.x = std::min(viewport->Size.x, mainViewport->Size.x);
            viewport->Size.y = std::min(viewport->Size.y, mainViewport->Size.y);
        }

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

bool GuiSystem::WantCaptureMouse() const {
    return m_wantCaptureMouse;
}

bool GuiSystem::WantCaptureKeyboard() const {
    return m_wantCaptureKeyboard;
}

void GuiSystem::OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!m_initialized) return;
    
    ImGui::SetCurrentContext(m_imguiContext);
    ImGui_ImplWin32_WndProcHandler(m_hwnd, msg, wParam, lParam);
    
    // Обновляем флаги сразу после обработки сообщения
    ImGuiIO& io = ImGui::GetIO();
    m_wantCaptureMouse = io.WantCaptureMouse;
    m_wantCaptureKeyboard = io.WantCaptureKeyboard;
}

void GuiSystem::ShutdownImGui() {
    if (m_imguiContext) {
        ImGui::SetCurrentContext(m_imguiContext);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(m_imguiContext);
        m_imguiContext = nullptr;
        Logger::Info("ImGui shutdown");
    }
}

void GuiSystem::Shutdown() {
    ShutdownImGui();
    m_initialized = false;
    Logger::Info("GuiSystem shutdown");
}

void GuiSystem::OnResize(int width, int height) {
    // ImGui сам обрабатывает через сообщения
}

void GuiSystem::DrawTestUI() {
    // Простейшее тестовое окно
    ImGui::Begin("OGLE Debug");
    ImGui::Text("Hello from ImGui!");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

} // namespace ogle