#include "App.h"

#include "Logger.h"
#include "core/Events.h"
#include "core/FileSystem.h"
#include "core/Layer.h"
#include "core/ExampleLayer.h"
#include "editor/EditorAssetHelpers.h"
#include "ui/IWindow.h"
#include <glm/vec3.hpp>
#include <entt/entt.hpp>
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
        auto& editor = m_app.GetEditor();
        auto& imguiManager = m_app.GetImGuiManager();
        auto& inputActionsManager = m_app.GetInputActionsManager();
        auto& cameraManager = m_app.GetCameraManager();
        auto& scriptManager = m_app.GetScriptManager();
        auto& physicsManager = m_app.GetPhysicsManager();
        auto& worldManager = m_app.GetWorldManager();
        
        if (!imguiManager.WantsKeyboardCapture() && !imguiManager.WantsMouseCapture()) {
            inputActionsManager.UpdateCameraControls(cameraManager, deltaTime);
        }

        const bool runSimulation = editor.GetSimulationState() == Editor::SimulationState::Playing;
        const bool stepSimulation = editor.ConsumeSimulationStepRequest();
        if (runSimulation || stepSimulation) {
            const float simulationDeltaTime = stepSimulation ? (1.0f / 60.0f) : deltaTime;
            scriptManager.Update(simulationDeltaTime);
            physicsManager.Update(simulationDeltaTime);
            worldManager.Update(simulationDeltaTime);
        }
        cameraManager.Update(deltaTime);
    }

    void OnImGuiRender() override
    {
        auto& eventBus = m_app.GetEventBus();
        m_app.GetEditor().GetState().eventBus = &eventBus;

        auto& cameraManager = m_app.GetCameraManager();
        auto& worldManager = m_app.GetWorldManager();
        auto& timeManager = m_app.GetTimeManager();
        auto& imguiManager = m_app.GetImGuiManager();
        auto& editor = m_app.GetEditor();
        auto& physicsManager = m_app.GetPhysicsManager();
        auto& configManager = m_app.GetConfigManager();
        auto& renderManager = m_app.GetRenderManager();

        imguiManager.BuildDefaultUi(cameraManager, worldManager, timeManager.GetDeltaTime());
        editor.BuildUi(cameraManager, worldManager, physicsManager, configManager);
        renderManager.SetHighlightedEntity(editor.GetSelectedEntity());
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

    if (config.world.loadOnStartup && FileSystem::Exists(worldPath)) {
        m_worldManager.LoadActiveWorld(worldPath.string());
        LOG_INFO("Loaded world from config: " + worldPath.string());
        return;
    }

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

    m_cameraManager.SetPosition(glm::vec3(7.0f, 5.0f, 9.0f));
    m_cameraManager.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    if (!m_renderManager.Initialize(*m_window, m_cameraManager, m_worldManager)) {
        return -1;
    }

    if (!m_imguiManager.Initialize(*m_window)) {
        LOG_ERROR("ImGui initialization failed");
        return -1;
    }

    if (!m_editor.Initialize()) {
        LOG_ERROR("Editor initialization failed");
        return -1;
    }
    m_editor.SetEnabled(m_configManager.GetConfig().editor.enabled);

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
        !config.scripts.startupScriptPath.empty() &&
        !m_scriptManager.ExecuteFile(config.scripts.startupScriptPath)) {
        LOG_WARN("Startup script was not executed");
    }

    // Push layers onto the stack
    m_layerStack.PushLayer(new MainApplicationLayer(*this));
    m_layerStack.PushLayer(new ExampleLayer());

    OGLE::TextureManager::Get().Initialize();
    m_window->Show(nCmdShow);
    m_timeManager.Reset();

    // ── Editor event subscriptions ─────────────────────────────────────
    m_eventBus.Subscribe<OGLE::EditorLoadWorldEvent>([this](const OGLE::EditorLoadWorldEvent& e) {
        m_configManager.GetConfig().world.path = e.path;
        m_configManager.Save();
        m_worldManager.LoadActiveWorld(e.path);
        m_editor.GetState().selectedEntity = entt::null;
        m_editor.GetState().bufferedEntity = entt::null;
        m_editor.GetState().textureEditingEntity = entt::null;
    });

    m_eventBus.Subscribe<OGLE::EditorSaveWorldEvent>([this](const OGLE::EditorSaveWorldEvent& e) {
        m_configManager.GetConfig().world.path = e.path;
        m_configManager.Save();
        m_worldManager.SaveActiveWorld(e.path);
    });

    m_eventBus.Subscribe<OGLE::EditorReloadDefaultWorldEvent>([this](const OGLE::EditorReloadDefaultWorldEvent&) {
        m_worldManager.CreateDefaultWorld();
        m_editor.GetState().selectedEntity = entt::null;
        m_editor.GetState().bufferedEntity = entt::null;
        m_editor.GetState().textureEditingEntity = entt::null;
    });

    m_eventBus.Subscribe<OGLE::EditorClearWorldEvent>([this](const OGLE::EditorClearWorldEvent&) {
        m_worldManager.ClearWorld();
        m_editor.GetState().selectedEntity = entt::null;
        m_editor.GetState().bufferedEntity = entt::null;
        m_editor.GetState().textureEditingEntity = entt::null;
    });

    m_eventBus.Subscribe<OGLE::EditorPlayEvent>([this](const OGLE::EditorPlayEvent&) {
        m_editor.GetState().simulationState = Editor::SimulationState::Playing;
    });

    m_eventBus.Subscribe<OGLE::EditorPauseEvent>([this](const OGLE::EditorPauseEvent&) {
        m_editor.GetState().simulationState = Editor::SimulationState::Paused;
    });

    m_eventBus.Subscribe<OGLE::EditorStepEvent>([this](const OGLE::EditorStepEvent&) {
        m_editor.GetState().simulationState = Editor::SimulationState::Paused;
        m_editor.GetState().stepSimulationRequested = true;
    });

    m_eventBus.Subscribe<OGLE::EditorCreateEntityEvent>([this](const OGLE::EditorCreateEntityEvent& e) {
        entt::entity created = entt::null;

        switch (e.type) {
            case OGLE::EditorCreateEntityEvent::Type::EmptyObject:
                created = m_worldManager.CreateWorldObject(e.name, OGLE::WorldObjectKind::Generic).GetEntity();
                break;
            case OGLE::EditorCreateEntityEvent::Type::Cube:
                created = m_worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Cube,
                    glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), e.texturePath);
                break;
            case OGLE::EditorCreateEntityEvent::Type::Sphere:
                created = m_worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Sphere,
                    glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), e.texturePath);
                break;
            case OGLE::EditorCreateEntityEvent::Type::Plane:
                created = m_worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Plane,
                    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(5.0f, 1.0f, 5.0f), e.texturePath);
                break;
            case OGLE::EditorCreateEntityEvent::Type::DirectionalLight:
                created = m_worldManager.CreateDirectionalLight(e.name, glm::vec3(-50.0f, 45.0f, 0.0f));
                break;
            case OGLE::EditorCreateEntityEvent::Type::PointLight:
                created = m_worldManager.CreatePointLight(e.name, glm::vec3(0.0f, 1.5f, 0.0f));
                break;
            case OGLE::EditorCreateEntityEvent::Type::ModelFromFile:
                created = m_worldManager.CreateModelFromFile(e.modelPath, OGLE::ModelType::DYNAMIC, e.name);
                if (created != entt::null && !e.texturePath.empty()) {
                    m_worldManager.SetEntityDiffuseTexture(created, e.texturePath);
                }
                break;
        }

        if (created != entt::null) {
            m_editor.GetState().selectedEntity = created;
            m_editor.GetState().bufferedEntity = entt::null;
            m_editor.GetState().textureEditingEntity = entt::null;
        }
    });

    m_eventBus.Subscribe<OGLE::EditorDeleteEntityEvent>([this](const OGLE::EditorDeleteEntityEvent& e) {
        if (e.entity != entt::null && m_worldManager.IsEntityValid(e.entity)) {
            m_physicsManager.RemoveBody(e.entity);
            m_worldManager.GetActiveWorld().DestroyEntity(e.entity);
        }
        if (e.entity == m_editor.GetState().selectedEntity) {
            m_editor.GetState().selectedEntity = entt::null;
            m_editor.GetState().bufferedEntity = entt::null;
            m_editor.GetState().textureEditingEntity = entt::null;
        }
    });

    m_eventBus.Subscribe<OGLE::EditorSpawnModelFromDragDropEvent>([this](const OGLE::EditorSpawnModelFromDragDropEvent& e) {
        // Reuse camera + spawn logic from the original drag-drop handler
        const auto& camera = m_cameraManager.GetCamera();
        const glm::vec3 spawnPosition = camera.GetPosition() + camera.GetFront() * 5.0f;
        const OGLE::Entity entity = m_worldManager.CreateModelFromFile(
            e.assetPath, OGLE::ModelType::DYNAMIC, BuildEditorEntityNameFromAssetPath(e.assetPath));

        if (entity != entt::null) {
            auto& world = m_worldManager.GetActiveWorld();
            if (auto* transform = world.GetComponent<OGLE::TransformComponent>(entity)) {
                world.SetTransform(entity, spawnPosition, transform->rotation, transform->scale);
            }
            m_editor.GetState().selectedEntity = entity;
            m_editor.GetState().bufferedEntity = entt::null;
            m_editor.GetState().textureEditingEntity = entt::null;
        }
    });

    m_eventBus.Subscribe<OGLE::EditorTransformChangedEvent>([this](const OGLE::EditorTransformChangedEvent& e) {
        if (e.entity != entt::null && m_worldManager.IsEntityValid(e.entity)) {
            m_worldManager.GetWorldObject(e.entity).SetTransform(e.position, e.rotation, e.scale);
        }
    });

    m_eventBus.Subscribe<OGLE::EditorNameChangedEvent>([this](const OGLE::EditorNameChangedEvent& e) {
        if (e.entity != entt::null && m_worldManager.IsEntityValid(e.entity)) {
            m_worldManager.GetWorldObject(e.entity).SetName(e.name);
        }
    });

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
