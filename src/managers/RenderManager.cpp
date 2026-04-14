#include "managers/RenderManager.h"

#include "managers/CameraManager.h"
#include "managers/ImGuiManager.h"
#include "managers/WorldManager.h"
#include "Logger.h"
#include "opengl/OpenGLInitializer.h"
#include "opengl/OpenGLRenderer.h"
#include "ui/IWindow.h"

#include <GL/gl.h>

RenderManager::RenderManager() = default;

RenderManager::~RenderManager() = default;

bool RenderManager::Initialize(IWindow& window, CameraManager& cameraManager, WorldManager& worldManager)
{
    RECT clientRect{};
    GetClientRect(window.Handle(), &clientRect);
    m_viewportWidth = clientRect.right - clientRect.left;
    m_viewportHeight = clientRect.bottom - clientRect.top;

    m_glInitializer = std::make_unique<OpenGLInitializer>(window.Handle(), window.GetDeviceContext());
    if (!m_glInitializer->Initialize(false))
    {
        LOG_ERROR("OpenGL initialization failed");
        return false;
    }

    LOG_INFO("OpenGL initialized");

    cameraManager.SetPerspective(
        45.0f,
        (m_viewportWidth > 0 && m_viewportHeight > 0) ? static_cast<float>(m_viewportWidth) / m_viewportHeight : 1.0f,
        0.1f,
        1000.0f);

    m_renderer = std::make_unique<OpenGLRenderer>(
        m_viewportWidth,
        m_viewportHeight,
        cameraManager.GetCamera(),
        worldManager);
    if (!m_renderer->Initialize()) {
        LOG_ERROR("OpenGLRenderer initialization failed");
        return false;
    }

    return true;
}

void RenderManager::Resize(int width, int height, IWindow& window)
{
    m_viewportWidth = width;
    m_viewportHeight = height;

    if (m_renderer) {
        m_renderer->Resize(width, height);
    }
}

void RenderManager::RenderFrame(IWindow& window, ImGuiManager* imguiManager)
{
    RECT clientRect{};
    GetClientRect(window.Handle(), &clientRect);
    const int actualWidth = clientRect.right - clientRect.left;
    const int actualHeight = clientRect.bottom - clientRect.top;

    if (m_renderer && actualWidth > 0 && actualHeight > 0 &&
        (actualWidth != m_viewportWidth || actualHeight != m_viewportHeight)) {
        m_viewportWidth = actualWidth;
        m_viewportHeight = actualHeight;
        m_renderer->Resize(actualWidth, actualHeight);
    }

    if (m_renderer) {
        m_renderer->Render();
    } else {
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (imguiManager) {
        imguiManager->Render();
    }

    ::SwapBuffers(window.GetDeviceContext());
}

void RenderManager::SetHighlightedEntity(OGLE::Entity entity)
{
    if (m_renderer) {
        m_renderer->SetHighlightedEntity(entity);
    }
}

void RenderManager::SetShowGrid(bool show)
{
    if (m_renderer) {
        m_renderer->SetShowGrid(show);
    }
}

void RenderManager::SetSceneViewport(const glm::vec2& origin, const glm::vec2& size)
{
    if (m_renderer) {
        m_renderer->SetSceneViewport(origin, size);
    }
}
