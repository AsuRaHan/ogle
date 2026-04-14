#pragma once

#include "world/WorldComponents.h"

#include <glm/vec2.hpp>
#include <memory>

class IWindow;
class OpenGLInitializer;
class OpenGLRenderer;
class CameraManager;
class WorldManager;
class ImGuiManager;

class RenderManager
{
public:
    RenderManager();
    ~RenderManager();

    bool Initialize(IWindow& window, CameraManager& cameraManager, WorldManager& worldManager);
    void Resize(int width, int height, IWindow& window);
    void RenderFrame(IWindow& window, ImGuiManager* imguiManager = nullptr);
    void SetHighlightedEntity(OGLE::Entity entity);
    void SetShowGrid(bool show);
    void SetSceneViewport(const glm::vec2& origin, const glm::vec2& size);

private:
    int m_viewportWidth = 0;
    int m_viewportHeight = 0;
    std::unique_ptr<OpenGLInitializer> m_glInitializer;
    std::unique_ptr<OpenGLRenderer> m_renderer;
};
