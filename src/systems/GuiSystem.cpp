#include "GuiSystem.h"
#include "log/Logger.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl3.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

void GuiSystem::SetWindowHandle(HWND hwnd, HDC /*hdc*/) {  // hdc пока не нужен
    if (m_hwnd) {
        Logger::Warning("GuiSystem window handle already set");
        return;
    }
    m_hwnd = hwnd;
    Logger::Debug("GuiSystem window handle set");
}

bool GuiSystem::Initialize() {
    if (m_initialized) {
        Logger::Warning("GuiSystem already initialized");
        return true;
    }

    Logger::Info("GuiSystem initializing...");

    if (!m_hwnd) {
        Logger::Error("GuiSystem: No HWND provided. Call SetWindowHandle() first!");
        return false;
    }

    return InitImGui();
}

bool GuiSystem::InitImGui() {
    IMGUI_CHECKVERSION();

    m_imguiContext = ImGui::CreateContext();
    if (!m_imguiContext) {
        Logger::Error("ImGui::CreateContext failed");
        return false;
    }

    ImGui::SetCurrentContext(m_imguiContext);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // если нужен docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMinSize = ImVec2(100.0f, 100.0f);
    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(m_hwnd)) {
        Logger::Error("ImGui_ImplWin32_Init failed");
        ImGui::DestroyContext(m_imguiContext);
        m_imguiContext = nullptr;
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 460 core")) {
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
    if (!m_initialized || !m_imguiContext) return;

    ImGui::SetCurrentContext(m_imguiContext);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    UIController::Get().Render();
    DrawTestUI();

    ImGuiIO& io = ImGui::GetIO();
    m_wantCaptureMouse    = io.WantCaptureMouse;
    m_wantCaptureKeyboard = io.WantCaptureKeyboard;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Если позже включишь ViewportsEnable — раскомментируй:
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     ImGui::UpdatePlatformWindows();
    //     ImGui::RenderPlatformWindowsDefault();
    // }
}

bool GuiSystem::WantCaptureMouse() const    { return m_wantCaptureMouse; }
bool GuiSystem::WantCaptureKeyboard() const { return m_wantCaptureKeyboard; }

void GuiSystem::OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!m_initialized || !m_imguiContext) return;

    ImGui::SetCurrentContext(m_imguiContext);

    ImGui_ImplWin32_WndProcHandler(m_hwnd, msg, wParam, lParam);

    // Флаги лучше обновлять в RenderUI после NewFrame, но можно и здесь
}

void GuiSystem::ShutdownImGui() {
    if (!m_imguiContext) return;

    ImGui::SetCurrentContext(m_imguiContext);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext(m_imguiContext);
    m_imguiContext = nullptr;

    Logger::Info("ImGui shutdown");
}

void GuiSystem::Shutdown() {
    ShutdownImGui();
    m_initialized = false;
    Logger::Info("GuiSystem shutdown");
}

void GuiSystem::OnResize(int /*width*/, int /*height*/) {
    // ImGui сам узнаёт размер через Win32 сообщения
}

void GuiSystem::DrawTestUI() {
    ImGui::Begin("OGLE Debug");
    ImGui::Text("Hello from ImGui!");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

} // namespace ogle