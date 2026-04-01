#pragma once

#include <string>

class IWindow;
class CameraManager;
class WorldManager;

class ImGuiManager
{
public:
    ImGuiManager() = default;
    ~ImGuiManager();

    bool Initialize(IWindow& window);
    void Shutdown();

    void BeginFrame();
    void BuildDefaultUi(const CameraManager& cameraManager, const WorldManager& worldManager, float deltaTime);
    void Render();

    bool WantsKeyboardCapture() const;
    bool WantsMouseCapture() const;

private:
    bool m_initialized = false;
    bool m_showDemoWindow = false;
    bool m_showOverlay = false;
};
