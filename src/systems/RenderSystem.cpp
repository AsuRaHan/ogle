// src/systems/RenderSystem.cpp
#include "RenderSystem.h"
#include "managers/CameraManager.h"
#include <algorithm>

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

		// Инициализация тестового куба
		if (!m_testCube.Initialize()) {
			Logger::Error("Failed to initialize TestCube");
			return false;
		}

		// Камера
		auto& cameraMgr = CameraManager::Get();
		m_camera = cameraMgr.GetMainCamera();

		if (!m_camera) {
			m_camera = cameraMgr.CreateCamera("MainCamera");
		}

		if (m_camera) {
			m_camera->SetPosition({ 2.0f, 2.0f, 3.0f });
			m_camera->LookAt({ 0.0f, 0.0f, 0.0f });
			m_camera->SetMode(Camera::Mode::Free);
			m_camera->SetPerspective(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
		}

		m_scene = std::make_unique<Scene>();
		m_scene->Initialize(m_camera);  // Передаём камеру для culling

		Logger::Info("RenderSystem initialized: " + m_context->GetVersionString());
		return true;
	}

	void RenderSystem::Update(float deltaTime) {
		// Обновляем тестовый куб
		//m_testCube.Update(deltaTime);
		m_time += deltaTime;
		// Обновляем сцену
		if (m_scene) {
			m_scene->Update(deltaTime);
		}

		// Обновляем камеру
		if (m_camera) {
			m_camera->Update(deltaTime);
		}

		//static float shaderCheckTimer = 0.0f;
		//shaderCheckTimer += deltaTime;
		//if (shaderCheckTimer >= 1.0f) { // Проверяем каждую секунду
			//ShaderController::Get().CheckForUpdates();
		//	shaderCheckTimer = 0.0f;
		//}
	}

	void RenderSystem::Render() {
		if (!m_context || !m_camera) return;

		m_context->MakeCurrent();

		// 1. Очистка
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//m_camera->Update(m_time); // Обновляем матрицы камеры перед рендером
		// 2. Рендерим 3D сцену
		//for (auto* renderer : m_renderers) {
		//	if (renderer) renderer->Render();
		//}

		m_testCube.Render(m_time, m_camera);

		//m_scene->Render(m_time, m_camera);

		// 3. Рендерим GUI поверх всего
		if (m_guiEnabled && m_guiSystem) {
			m_guiSystem->RenderUI();  // ВСЁ в одном методе!
		}

		m_context->SwapBuffers();
	}

	void RenderSystem::OnResize(int width, int height) {
		if (!m_context || width <= 0 || height <= 0) return;

		m_context->MakeCurrent();
		glViewport(0, 0, width, height);

		if (m_camera && m_camera->GetType() == Camera::Type::Perspective) {
			m_camera->SetAspectRatio(static_cast<float>(width) / height);
		}

		Logger::Debug("Viewport resized: " + std::to_string(width) + "x" + std::to_string(height));
	}

	void RenderSystem::Shutdown() {
		m_scene.reset();
		m_context.reset();
		m_camera = nullptr;
		m_renderers.clear();
		m_guiSystem = nullptr;
		Logger::Info("RenderSystem shutdown");
	}

	// === Управление рендерерами ===

	void RenderSystem::AddRenderer(ISystem* system) {
		if (!system) return;

		// Проверяем, нет ли уже такой системы
		auto it = std::find(m_renderers.begin(), m_renderers.end(), system);
		if (it == m_renderers.end()) {
			m_renderers.push_back(system);
			Logger::Debug("Renderer added: " + system->GetName());
		}
	}

	void RenderSystem::RemoveRenderer(ISystem* system) {
		auto it = std::find(m_renderers.begin(), m_renderers.end(), system);
		if (it != m_renderers.end()) {
			m_renderers.erase(it);
			Logger::Debug("Renderer removed: " + system->GetName());
		}
	}

	void RenderSystem::ClearRenderers() {
		m_renderers.clear();
		Logger::Debug("All renderers cleared");
	}

	// === GUI ===

	void RenderSystem::SetGuiSystem(GuiSystem* guiSystem) {
		m_guiSystem = guiSystem;
		Logger::Debug("GUI system set: " +
			std::string(guiSystem ? guiSystem->GetName() : "nullptr"));
	}

} // namespace ogle