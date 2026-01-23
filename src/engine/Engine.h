#pragma once

#include "window/OpenGLContext.h"
#include "engine/Renderer.h"
#include "engine/Scene.h"
#include "engine/Camera.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>  // std::unique_ptr

class Engine
{
public:
    Engine(HINSTANCE hInstance);
    ~Engine();

    bool Initialize();
    int Run();
    void Shutdown();

private:
    HINSTANCE m_hInstance;
    std::unique_ptr<MainWindow> m_window;
    std::unique_ptr<OpenGLContext> m_glContext;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Camera> m_camera;

    float m_aspectRatio = 1.0f;  // кэшируем, обновляем только при ресайзе

    bool m_running { false };
    LARGE_INTEGER m_frequency {};
    LARGE_INTEGER m_lastTime {};

    void ProcessInput();
    void Update(double deltaTime);
    void Render();
};