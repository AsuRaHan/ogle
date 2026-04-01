#include "managers/ImGuiManager.h"

#include "Logger.h"
#include "managers/CameraManager.h"
#include "managers/WorldManager.h"
#include "ui/IWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ImGuiManager::~ImGuiManager()
{
    Shutdown();
}

bool ImGuiManager::Initialize(IWindow& window)
{
    Shutdown();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(window.Handle())) {
        LOG_ERROR("ImGui Win32 initialization failed");
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
        LOG_ERROR("ImGui OpenGL3 initialization failed");
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    const HWND hwnd = window.Handle();
    window.AddMessageObserver([hwnd](UINT msg, WPARAM wParam, LPARAM lParam) {
        ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
    });

    m_initialized = true;
    LOG_INFO("ImGui initialized");
    return true;
}

void ImGuiManager::Shutdown()
{
    if (!m_initialized) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

void ImGuiManager::BeginFrame()
{
    if (!m_initialized) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::BuildDefaultUi(const CameraManager& cameraManager, const WorldManager& worldManager, float deltaTime)
{
    if (!m_initialized) {
        return;
    }

#ifdef IMGUI_HAS_DOCK
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockspaceWindowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("MainDockSpace", nullptr, dockspaceWindowFlags)) {
        ImGui::PopStyleVar(3);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Workspace")) {
                ImGui::MenuItem("Docking Enabled", nullptr, true, false);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGuiID dockspaceId = ImGui::GetID("OGLEMainDockSpace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    } else {
        ImGui::PopStyleVar(3);
    }
    ImGui::End();
#endif

    if (m_showOverlay) {
        const auto& camera = cameraManager.GetCamera();
        const glm::vec3& position = camera.GetPosition();
        const glm::vec3& front = camera.GetFront();

        ImGui::Begin("OGLE Debug");
        ImGui::Text("FPS: %.1f", deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f);
        ImGui::Text("Frame dt: %.4f", deltaTime);
        ImGui::Separator();
        ImGui::Text("Camera position: %.2f %.2f %.2f", position.x, position.y, position.z);
        ImGui::Text("Camera front: %.2f %.2f %.2f", front.x, front.y, front.z);
        std::size_t entityCount = 0;
        const auto entityView = worldManager.GetActiveWorld().GetRegistry().view<OGLE::NameComponent>();
        for (auto entity : entityView) {
            (void)entity;
            ++entityCount;
        }
        ImGui::Text("World entities: %u", static_cast<unsigned int>(entityCount));
        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::BulletText("W A S D / Q E move camera");
        ImGui::BulletText("Right mouse button rotates camera");
        ImGui::BulletText("Shift speeds up movement");
        ImGui::Checkbox("Show Dear ImGui Demo", &m_showDemoWindow);
        ImGui::End();
    }

    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
}

void ImGuiManager::Render()
{
    if (!m_initialized) {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiManager::WantsKeyboardCapture() const
{
    return m_initialized && ImGui::GetIO().WantCaptureKeyboard;
}

bool ImGuiManager::WantsMouseCapture() const
{
    return m_initialized && ImGui::GetIO().WantCaptureMouse;
}
