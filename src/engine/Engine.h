// Engine.h
#pragma once

#include "window/OpenGLContext.h"
#include "engine/Renderer.h"
#include "engine/Scene.h"
#include "engine/Camera.h"
#include "engine/input/InputSystem.h"  // Используем новую систему ввода
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>

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

    // Контекст ввода для геймплея
    ogle::input::InputContext* m_gameplayContext = nullptr;
    
    float m_aspectRatio = 1.0f;
    float m_lastDeltaTime = 0.0f;  // Для доступа в коллбеках

    bool m_running { false };
    LARGE_INTEGER m_frequency {};
    LARGE_INTEGER m_lastTime {};

    void HandleWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void Update(double deltaTime);
    void Render();
    
    // Вспомогательные методы для настройки ввода
    void SetupInputBindings();
    void UpdateCameraFromInput(float deltaTime);
};