// src/systems/RenderSystem.cpp
#include "RenderSystem.h"
#include "managers/CameraManager.h"  // Добавляем

namespace ogle {

RenderSystem::RenderSystem(HDC hdc) : m_hdc(hdc) {}

RenderSystem::~RenderSystem() = default;

bool RenderSystem::Initialize() {
    if (!m_hdc) {
        Logger::Error("RenderSystem: invalid HDC");
        return false;
    }

    m_context = std::make_unique<OpenGLContext>(m_hdc);

    if (!m_context->Initialize(4, 6, true)) {
        Logger::Error("OpenGLContext init failed");
        return false;
    }

    m_context->MakeCurrent();

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);

    // Начальный viewport
    OnResize(1280, 720);

    Logger::Info("RenderSystem initialized: " + m_context->GetVersionString());

    // Инициализация тестового куба
    if (!m_testCube.Initialize()) {
        Logger::Error("Failed to initialize TestCube");
        return false;
    }
    
    // ======== ВОТ ТУТ СОЗДАЕМ/ПОЛУЧАЕМ КАМЕРУ ========
    auto& cameraMgr = CameraManager::Get();
    
    // Получаем основную камеру
    m_camera = cameraMgr.GetMainCamera();
    
    // Если камеры нет - создаем
    if (!m_camera) {
        m_camera = cameraMgr.CreateCamera("MainCamera");
        Logger::Info("Main camera created");
    }
    
    // Настраиваем камеру
    if (m_camera) {
        m_camera->SetPosition({ 2.0f, 2.0f, 3.0f });
        m_camera->LookAt({ 0.0f, 0.0f, 0.0f });
        m_camera->SetMode(Camera::Mode::Free);
        m_camera->SetPerspective(45.0f, 1280.0f/720.0f, 0.1f, 100.0f);
        
        Logger::Info("Camera configured");
    } else {
        Logger::Error("Failed to get or create camera");
        return false;
    }
    
    return true;
}

void RenderSystem::Update(float deltaTime) {
    // Сохраняем deltaTime для обновления времени анимации
    m_time += deltaTime;
    
    // Обновляем камеру (если она анимирована или управляется)
    if (m_camera) {
        m_camera->Update(deltaTime);
    }
}

void RenderSystem::Render() {
    if (!m_context || !m_camera) return;
    
    m_context->MakeCurrent();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Рендерим куб с матрицами из нашей камеры
    m_testCube.Render(
        m_time, 
        m_camera->GetViewMatrix(), 
        m_camera->GetProjectionMatrix()
    );

    m_context->SwapBuffers();
}

void RenderSystem::OnResize(int width, int height) {
    if (!m_context || width <= 0 || height <= 0) return;

    m_context->MakeCurrent();
    glViewport(0, 0, width, height);
    
    // Обновляем аспектное соотношение камеры
    if (m_camera && m_camera->GetType() == Camera::Type::Perspective) {

        
        m_camera->SetAspectRatio(static_cast<float>(width) / height);
        
        
        // Пока просто логируем
        Logger::Debug("Camera aspect ratio should be updated to: " + 
            std::to_string(static_cast<float>(width) / height));
    }
    
    Logger::Debug("Viewport resized: " + 
        std::to_string(width) + "x" + std::to_string(height));
}

void RenderSystem::Shutdown() {
    m_context.reset();
    m_camera = nullptr;  // Не удаляем, менеджер сам управляет
    Logger::Info("RenderSystem shutdown");
}

void RenderSystem::SetClearColor(float r, float g, float b, float a) {
    m_clearColor[0] = r;
    m_clearColor[1] = g;
    m_clearColor[2] = b;
    m_clearColor[3] = a;
}

} // namespace ogle