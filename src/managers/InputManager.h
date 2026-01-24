#pragma once
#include <windows.h>
#include "log/Logger.h"

namespace ogle {

	class InputManager {
	public:
		static InputManager& Get() {
			static InputManager instance;
			return instance;
		}

		// Простая заглушка: обработка клавиши Esc
		void ProcessKey(int keyCode) {
			if (keyCode == VK_ESCAPE) {
				// Выход из приложения
				PostQuitMessage(0);
				Logger::Info("Escape pressed - quitting application");
			}
		}

		// Пустые методы для будущего
		void Update() {}
		void Initialize() {}

	private:
		InputManager() = default;
		~InputManager() = default;
	};

} // namespace ogle