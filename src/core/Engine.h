// Engine.h
#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>

#include "window/OpenGLContext.h"
#include "render/Renderer.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "input/InputSystem.h"
#include "input/InputConfigurator.h"


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
    
    // Конфигуратор ввода вместо прямого управления
    std::unique_ptr<ogle::InputConfigurator> m_inputConfigurator;
    
    float m_aspectRatio = 1.0f;
    float m_lastDeltaTime = 0.0f;

    bool m_running { false };
    LARGE_INTEGER m_frequency {};
    LARGE_INTEGER m_lastTime {};

    void HandleWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void Update(double deltaTime);
    void Render();
};