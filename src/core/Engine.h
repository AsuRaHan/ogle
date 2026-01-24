// src/core/Engine.h
#pragma once

#include <memory>
#include <vector>
#include <windows.h>

#include "ISystem.h"
#include "log/Logger.h"
#include "systems/WindowSystem.h"
#include "systems/RenderSystem.h"
#include "systems/InputSystem.h"

namespace ogle {

	class Engine {
	public:
		explicit Engine(HINSTANCE hInstance);
		~Engine();

		bool Initialize();
		int Run();
		void Shutdown();

		template<typename T, typename... Args>
		T* RegisterSystem(Args&&... args);

		template<typename T>
		T* GetSystem() const;

		float GetDeltaTime() const { return m_deltaTime; }

	private:
		HINSTANCE m_hInstance;

		std::vector<std::unique_ptr<ISystem>> m_systems;

		LARGE_INTEGER m_frequency{};
		LARGE_INTEGER m_lastTime{};
		float m_deltaTime = 0.0f;
		bool m_running = false;

		void ProcessMessages();
		void UpdateSystems();
		void RenderSystems();
	};

	template<typename T, typename... Args>
	T* Engine::RegisterSystem(Args&&... args) {
		auto system = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = system.get();
		m_systems.push_back(std::move(system));
		Logger::Info("Registered system: " + ptr->GetName());
		return ptr;
	}

	template<typename T>
	T* Engine::GetSystem() const {
		for (const auto& sys : m_systems) {
			if (auto* casted = dynamic_cast<T*>(sys.get())) {
				return casted;
			}
		}
		return nullptr;
	}

} // namespace ogle