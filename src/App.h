#pragma once

#include "config/ConfigManager.h"
#include "core/LayerStack.h"
#include "editor/Editor.h"
#include "managers/CameraManager.h"
#include "managers/ImGuiManager.h"
#include "managers/InputActionsManager.h"
#include "managers/InputManager.h"
#include "managers/PhysicsManager.h"
#include "managers/RenderManager.h"
#include "managers/ScriptManager.h"
#include "managers/TimeManager.h"
#include "managers/WorldManager.h"
#include <memory>
#include <windows.h>

class IWindow;

class App
{
public:
    App(std::unique_ptr<IWindow> window, ConfigManager configManager);
    int Run(HINSTANCE hInstance, int nCmdShow);

    static App* Get() { return s_instance; }

    // Getters for all managers
    IWindow& GetWindow() { return *m_window; }
    const IWindow& GetWindow() const { return *m_window; }
    ConfigManager& GetConfigManager() { return m_configManager; }
    const ConfigManager& GetConfigManager() const { return m_configManager; }
    Editor& GetEditor() { return m_editor; }
    const Editor& GetEditor() const { return m_editor; }
    CameraManager& GetCameraManager() { return m_cameraManager; }
    const CameraManager& GetCameraManager() const { return m_cameraManager; }
    ImGuiManager& GetImGuiManager() { return m_imguiManager; }
    const ImGuiManager& GetImGuiManager() const { return m_imguiManager; }
    InputActionsManager& GetInputActionsManager() { return m_inputActionsManager; }
    const InputActionsManager& GetInputActionsManager() const { return m_inputActionsManager; }
    InputManager& GetInputManager() { return m_inputManager; }
    const InputManager& GetInputManager() const { return m_inputManager; }
    PhysicsManager& GetPhysicsManager() { return m_physicsManager; }
    const PhysicsManager& GetPhysicsManager() const { return m_physicsManager; }
    TimeManager& GetTimeManager() { return m_timeManager; }
    const TimeManager& GetTimeManager() const { return m_timeManager; }
    RenderManager& GetRenderManager() { return m_renderManager; }
    const RenderManager& GetRenderManager() const { return m_renderManager; }
    ScriptManager& GetScriptManager() { return m_scriptManager; }
    const ScriptManager& GetScriptManager() const { return m_scriptManager; }
    WorldManager& GetWorldManager() { return m_worldManager; }
    const WorldManager& GetWorldManager() const { return m_worldManager; }

    void SaveWindowState(const std::string& filePath);

private:
    void InitializeWorldFromConfig();

    static App* s_instance;

    LayerStack m_layerStack;
    std::unique_ptr<IWindow> m_window;
    ConfigManager m_configManager;
    Editor m_editor;
    CameraManager m_cameraManager;
    ImGuiManager m_imguiManager;
    InputActionsManager m_inputActionsManager;
    InputManager m_inputManager;
    PhysicsManager m_physicsManager;
    TimeManager m_timeManager;
    RenderManager m_renderManager;
    ScriptManager m_scriptManager;
    WorldManager m_worldManager;
};
