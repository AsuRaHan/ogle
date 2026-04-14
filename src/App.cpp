#include "App.h"

#include "Logger.h"
#include "core/Events.h"
#include "core/FileSystem.h"
#include "core/Layer.h"
#include "core/ExampleLayer.h"
#include "opengl/Camera.h"
#include "ui/IWindow.h"
#include <glm/vec3.hpp>
#include <entt/entt.hpp>
#include <imgui.h>
#include <string>

// Initialize singleton instance
App* App::s_instance = nullptr;

namespace
{
// This layer encapsulates the original application logic to keep it running
// within the new layer-based system.
class MainApplicationLayer : public Layer
{
public:
    MainApplicationLayer(App& app)
        : Layer("MainApplicationLayer")
        , m_app(app)
    {
    }

    void OnUpdate(float deltaTime) override
    {
        auto& imguiManager = m_app.GetImGuiManager();
        auto& inputActionsManager = m_app.GetInputActionsManager();
        auto& cameraManager = m_app.GetCameraManager();
        auto& scriptManager = m_app.GetScriptManager();
        auto& physicsManager = m_app.GetPhysicsManager();
        auto& worldManager = m_app.GetWorldManager();    
        inputActionsManager.UpdateCameraControls(cameraManager, deltaTime);
        scriptManager.Update(deltaTime);
        physicsManager.Update(deltaTime);
        worldManager.Update(deltaTime);

        cameraManager.Update(deltaTime);
    }

    void OnImGuiRender() override
    {
        auto& eventBus = m_app.GetEventBus();
        auto& cameraManager = m_app.GetCameraManager();
        auto& worldManager = m_app.GetWorldManager();
        auto& timeManager = m_app.GetTimeManager();
        auto& imguiManager = m_app.GetImGuiManager();
        auto& physicsManager = m_app.GetPhysicsManager();
        auto& configManager = m_app.GetConfigManager();
        auto& renderManager = m_app.GetRenderManager();

        imguiManager.BuildDefaultUi(cameraManager, worldManager, timeManager.GetDeltaTime());

    }

private:
    App& m_app;
};
}

App::App(std::unique_ptr<IWindow> window, ConfigManager configManager)
    : m_window(std::move(window))
    , m_configManager(std::move(configManager))
{
    s_instance = this;
	LOG_INFO("App instance created");
}

void App::InitializeWorldFromConfig()
{
    const AppConfig& config = m_configManager.GetConfig();
    const std::filesystem::path worldPath = FileSystem::ResolvePath(config.world.path);

    // if (config.world.loadOnStartup && FileSystem::Exists(worldPath)) {
    //     m_worldManager.LoadActiveWorld(worldPath.string());
    //     LOG_INFO("Loaded world from config: " + worldPath.string());
    //     return;
    // }

    m_worldManager.CreateDefaultWorld();
    LOG_INFO("Created default world");

    if (config.world.saveDefaultWorldIfMissing) {
        m_worldManager.SaveActiveWorld(worldPath.string());
        LOG_INFO("Saved default world to: " + worldPath.string());
    }
}

int App::Run(HINSTANCE hInstance, int nCmdShow)
{
    if (!m_window || !m_window->Create(hInstance))
    {
        LOG_ERROR("Window creation failed");
        return -1;
    }

    LOG_INFO("Window created successfully");
    m_inputManager.AttachToWindow(*m_window);
    m_inputActionsManager.ConfigureDefaultActions();

    // m_cameraManager.SetPosition(glm::vec3(7.0f, 5.0f, 9.0f));
    // m_cameraManager.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    if (!m_renderManager.Initialize(*m_window, m_cameraManager, m_worldManager)) {
        return -1;
    }

    if (!m_imguiManager.Initialize(*m_window)) {
        LOG_ERROR("ImGui initialization failed");
        return -1;
    }

    // if (!m_editor.Initialize()) {
    //     LOG_ERROR("Editor initialization failed");
    //     return -1;
    // }
    // m_editor.SetEnabled(m_configManager.GetConfig().editor.enabled);

    InitializeWorldFromConfig();

    if (!m_physicsManager.Initialize(m_worldManager)) {
        LOG_ERROR("Physics system initialization failed");
        return -1;
    }

    m_physicsManager.SetCollisionCallback([this](entt::entity a, entt::entity b) {
        const auto eidA = static_cast<unsigned int>(entt::to_integral(a));
        const auto eidB = static_cast<unsigned int>(entt::to_integral(b));
        LOG_INFO("Collision detected between entities " + std::to_string(eidA) + " and " + std::to_string(eidB));
        m_eventBus.Dispatch(OGLE::CollisionEvent{a, b});
    });

    m_eventBus.Subscribe<OGLE::CollisionEvent>([this](const OGLE::CollisionEvent& e) {
        m_scriptManager.NotifyCollision(e.entityA, e.entityB);
    });

    if (!m_scriptManager.Initialize(m_worldManager, m_physicsManager, "assets/scripts/internal/api_bootstrap.js")) {
        LOG_ERROR("Script system initialization failed");
        return -1;
    }

    const AppConfig& config = m_configManager.GetConfig();
    if (config.scripts.runStartupScript &&
        !config.scripts.startupScriptPath.empty()) {
        const std::filesystem::path startupScriptPath = FileSystem::ResolvePath(config.scripts.startupScriptPath);
        if (FileSystem::Exists(startupScriptPath)) {
            if (!m_scriptManager.ExecuteFile(startupScriptPath.string())) {
                LOG_WARN("Startup script was not executed");
            }
        } else {
            LOG_WARN("Startup script not found, skipping: " + config.scripts.startupScriptPath);
        }
    }

    // Push layers onto the stack
    m_layerStack.PushLayer(new MainApplicationLayer(*this));
    // m_layerStack.PushLayer(new ExampleLayer());

    OGLE::TextureManager::Get().Initialize();
    m_window->Show(nCmdShow);
    m_timeManager.Reset();

    // m_editor.SubscribeToEvents(m_eventBus, m_configManager, m_worldManager, m_physicsManager, m_cameraManager);

    // Set up event subscriptions
    m_eventBus.Subscribe<OGLE::WindowResizeEvent>([this](const OGLE::WindowResizeEvent& e) {
        m_renderManager.Resize(e.width, e.height, *m_window);
    });

    MSG msg{};
    bool MainLoop = true;
    while (MainLoop) {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                MainLoop = false;
            }
            else if (msg.message == WM_SIZE) {
                int newWidth = LOWORD(msg.lParam);
                int newHeight = HIWORD(msg.lParam);
                m_eventBus.Dispatch(OGLE::WindowResizeEvent{newWidth, newHeight});
            }
        }

        if (!MainLoop) {
            break;
        }

        const float deltaTime = m_timeManager.Tick();
        m_inputManager.Update(deltaTime);

        // 1. Update phase
        for (auto* layer : m_layerStack)
            layer->OnUpdate(deltaTime);

        m_imguiManager.BeginFrame();

        // 2. ImGui rendering phase
        for (auto* layer : m_layerStack)
            layer->OnImGuiRender();

        // 3. Final render
        m_renderManager.RenderFrame(*m_window, &m_imguiManager);
    }

    LOG_INFO("Main loop exited");

    return static_cast<int>(msg.wParam);
}
