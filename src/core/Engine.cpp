// src/core/Engine.cpp
#include "Engine.h"

namespace ogle {

	Engine::Engine(HINSTANCE hInstance) : m_hInstance(hInstance) {}

	Engine::~Engine() {
		Shutdown();
	}

	bool Engine::Initialize() {
		QueryPerformanceFrequency(&m_frequency);
		QueryPerformanceCounter(&m_lastTime);

		Logger::Info("Engine initializing...");

		// 1. Окно
		auto* window = RegisterSystem<WindowSystem>(m_hInstance, L"OGLE 3D Engine", 1280, 720);
		if (!window || !window->InitWindow()) {
			Logger::Error("WindowSystem init failed");
			return false;
		}

		// 2. Ввод
		auto* input = RegisterSystem<InputSystem>();
		input->SetWindowHandle(window->GetHWND());
		if (!input->Initialize()) {
			Logger::Error("InputSystem initialization failed");
			return false;
		}

		// 3. Рендер
		auto* render = RegisterSystem<RenderSystem>(window->GetHDC());
		if (!render->Initialize()) {
			Logger::Error("RenderSystem initialization failed");
			return false;
		}

		// 4. GUI
		m_guiSystem = RegisterSystem<GuiSystem>();
		m_guiSystem->SetWindowHandle(window->GetHWND(), window->GetHDC());
		if (!m_guiSystem->Initialize()) {
			Logger::Error("GuiSystem initialization failed");
			return false;
		}

		// 5. Регистрируем GUI в RenderSystem
		render->SetGuiSystem(m_guiSystem);

		// 6. Регистрируем другие системы как рендереры (если нужно)
		// Пример: render->AddRenderer(someOtherSystem);

		// 7. Показываем окно
		window->Show(SW_SHOW);

		// Подписка на ресайз
		window->AddResizeListener([render](int w, int h) {
			render->OnResize(w, h);
			});

		m_running = true;
		Logger::Success("Engine initialized successfully");
		return true;
	}

	void Engine::UpdateSystems() {
		//UIController::Get().Update(m_deltaTime);
		for (auto& sys : m_systems) {
			sys->Update(m_deltaTime);
		}
	}

	void Engine::RenderSystems() {
		// Только RenderSystem рендерит
		auto* renderSystem = GetSystem<RenderSystem>();
		if (renderSystem) {
			renderSystem->Render();
		}
	}

	void Engine::Shutdown() {
		// Shutdown в обратном порядке
		for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
			(*it)->Shutdown();
		}
		m_systems.clear();

		// Очищаем GUI pointer
		m_guiSystem = nullptr;

		Logger::Info("Engine shutdown");
	}

	int Engine::Run() {
		MSG msg{};
		while (m_running) {
			ProcessMessages();
			if (!m_running) break;

			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			m_deltaTime = static_cast<float>(
				static_cast<double>(now.QuadPart - m_lastTime.QuadPart) / m_frequency.QuadPart);
			m_lastTime = now;

			UpdateSystems();
			RenderSystems();
		}

		return 0;
	}

	void Engine::ProcessMessages() {
		MSG msg{};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// Передаем сообщения в GUI
			if (m_guiSystem && m_guiSystem->IsInitialized()) {
				m_guiSystem->OnWindowMessage(msg.message, msg.wParam, msg.lParam);
			}

			// Проверяем, хочет ли GUI захватить ввод
			bool guiWantsInput = false;
			if (m_guiSystem && m_guiSystem->IsInitialized()) {
				guiWantsInput = m_guiSystem->WantCaptureMouse() ||
					m_guiSystem->WantCaptureKeyboard();
			}

			// Если GUI не захватил ввод - передаем остальным
			if (!guiWantsInput) {
				for (auto& sys : m_systems) {
					if (sys.get() != m_guiSystem) {
						sys->OnWindowMessage(msg.message, msg.wParam, msg.lParam);
					}
				}
			}

			if (msg.message == WM_QUIT) {
				m_running = false;
			}
		}
	}

} // namespace ogle