// src/systems/RenderSystem.cpp
#include "RenderSystem.h"

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
		return true;
	}

	void RenderSystem::Shutdown() {
		m_context.reset();
		Logger::Info("RenderSystem shutdown");
	}

	void RenderSystem::Render() {
		if (!m_context) return;
		m_context->MakeCurrent();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_time += 0.01f;  // ← обновляем время
		m_testCube.Render(m_time);

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