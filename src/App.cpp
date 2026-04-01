#include "App.h"

#include "Logger.h"
#include "ui/IWindow.h"
#include <glm/vec3.hpp>

App::App(std::unique_ptr<IWindow> window)
    : m_window(std::move(window))
{
}

CameraManager& App::GetCameraManager()
{
    return m_cameraManager;
}

const CameraManager& App::GetCameraManager() const
{
    return m_cameraManager;
}

Editor& App::GetEditor()
{
    return m_editor;
}

const Editor& App::GetEditor() const
{
    return m_editor;
}

WorldManager& App::GetWorldManager()
{
    return m_worldManager;
}

const WorldManager& App::GetWorldManager() const
{
    return m_worldManager;
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

    m_worldManager.CreateDefaultWorld();
    m_worldManager.SaveActiveWorld("world.json");

    if (!m_scriptManager.Initialize(m_worldManager)) {
        LOG_ERROR("Script system initialization failed");
        return -1;
    }

    if (!m_scriptManager.ExecuteFile("scripts/test_world.js")) {
        LOG_WARN("Test script was not executed");
    }

    if (!m_physicsManager.Initialize(m_worldManager)) {
        LOG_ERROR("Physics system initialization failed");
        return -1;
    }

    const OGLE::Entity floorEntity = m_worldManager.FindEntityByName("Floor");
    if (floorEntity != entt::null) {
        m_physicsManager.AddBoxBody(
            floorEntity,
            glm::vec3(12.0f, 0.25f, 12.0f),
            OGLE::PhysicsBodyType::Static,
            0.0f);
    }

    for (const char* entityName : { "CenterBlock", "WestBlock", "EastBlock", "MarkerA", "MarkerB" }) {
        const OGLE::Entity entity = m_worldManager.FindEntityByName(entityName);
        if (entity != entt::null) {
            const auto* transform = m_worldManager.GetActiveWorld().GetTransform(entity);
            if (transform) {
                m_physicsManager.AddBoxBody(
                    entity,
                    transform->scale * 0.5f,
                    OGLE::PhysicsBodyType::Dynamic,
                    1.0f);
            }
        }
    }

    const OGLE::Entity physicsDropA = m_worldManager.CreateCube(
        "PhysicsDropA",
        glm::vec3(-1.5f, 6.0f, 0.0f),
        glm::vec3(0.8f, 0.8f, 0.8f));
    m_physicsManager.AddBoxBody(
        physicsDropA,
        glm::vec3(0.4f, 0.4f, 0.4f),
        OGLE::PhysicsBodyType::Dynamic,
        1.0f);

    const OGLE::Entity physicsDropB = m_worldManager.CreateCube(
        "PhysicsDropB",
        glm::vec3(1.2f, 8.0f, -0.5f),
        glm::vec3(1.0f, 1.0f, 1.0f));
    m_physicsManager.AddBoxBody(
        physicsDropB,
        glm::vec3(0.5f, 0.5f, 0.5f),
        OGLE::PhysicsBodyType::Dynamic,
        1.0f);

    m_window->Show(nCmdShow);
    m_timeManager.Reset();

    MSG msg{};
    bool MainLoop = true;
    while (MainLoop) {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                MainLoop = false; // Завершение приложения
            }
            else if (msg.message == WM_SIZE) {
                int newWidth = LOWORD(msg.lParam);
                int newHeight = HIWORD(msg.lParam);
                m_renderManager.Resize(newWidth, newHeight, *m_window);
            }
        }

        if (!MainLoop) {
            break;
        }

        const float deltaTime = m_timeManager.Tick();
        m_inputManager.Update(deltaTime);
        m_imguiManager.BeginFrame();
        m_imguiManager.BuildDefaultUi(m_cameraManager, m_worldManager, deltaTime);
        m_editor.BuildUi(m_cameraManager, m_worldManager, m_physicsManager);

        if (!m_imguiManager.WantsKeyboardCapture() && !m_imguiManager.WantsMouseCapture()) {
            m_inputActionsManager.UpdateCameraControls(m_cameraManager, deltaTime);
        }

        m_scriptManager.Update(deltaTime);
        m_physicsManager.Update(deltaTime);
        m_worldManager.Update();
        m_cameraManager.Update(deltaTime);
        m_renderManager.RenderFrame(*m_window, &m_imguiManager);
    }

    LOG_INFO("Main loop exited");

    return static_cast<int>(msg.wParam);
}
