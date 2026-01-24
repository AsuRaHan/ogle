// src/systems/RenderSystem.cpp
#include "RenderSystem.h"


namespace ogle {

RenderSystem::RenderSystem(HDC hdc) : m_hdc(hdc) {
    if (!m_hdc) {
        Logger::Error("RenderSystem создан с invalid HDC — контекст не будет создан");
    }
}

RenderSystem::~RenderSystem() = default;

const std::string& RenderSystem::GetName() const {
    static const std::string name = "RenderSystem";
    return name;
}

bool RenderSystem::Initialize() {
    if (!m_hdc) {
        Logger::Error("RenderSystem: HDC не был передан в конструкторе");
        return false;
    }

    m_context = std::make_unique<OpenGLContext>(m_hdc);

    if (!m_context->Initialize(4, 6, true)) {  // 4.6 core + debug
        Logger::Error("Не удалось инициализировать OpenGL контекст");
        return false;
    }

    m_context->MakeCurrent();
    glEnable(GL_DEPTH_TEST);
    glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);

    // Начальный viewport — потом перезапишется через OnResize
    OnResize(1280, 720);

    Logger::Info("RenderSystem initialized: " + m_context->GetVersionString());
    return true;
}

void RenderSystem::Shutdown() {
    m_context.reset();
    Logger::Info("RenderSystem shutdown");
}

void RenderSystem::Render() {
    if (!m_context) return;

    m_context->MakeCurrent();

    glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Здесь позже будет рендер сцены / шейдеры / кубики

    m_context->SwapBuffers();
}

void RenderSystem::OnResize(int width, int height) {
    if (!m_context || width <= 0 || height <= 0) return;

    m_context->MakeCurrent();
    glViewport(0, 0, width, height);
    Logger::Debug("Viewport resized: " + std::to_string(width) + "x" + std::to_string(height));
}

void RenderSystem::SetClearColor(float r, float g, float b, float a) {
    m_clearColor[0] = r;
    m_clearColor[1] = g;
    m_clearColor[2] = b;
    m_clearColor[3] = a;
}

} // namespace ogle