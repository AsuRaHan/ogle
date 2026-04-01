#pragma once

#include "config/ConfigManager.h"
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
    CameraManager& GetCameraManager();
    const CameraManager& GetCameraManager() const;
    Editor& GetEditor();
    const Editor& GetEditor() const;
    ConfigManager& GetConfigManager();
    const ConfigManager& GetConfigManager() const;
    WorldManager& GetWorldManager();
    const WorldManager& GetWorldManager() const;

private:
    void InitializeWorldFromConfig();

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
