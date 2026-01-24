// src/core/ISystem.h
#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>   // UINT, WPARAM, LPARAM
#endif

namespace ogle {

	class ISystem {
	public:
		virtual ~ISystem() = default;

		virtual const std::string& GetName() const = 0;

		virtual bool Initialize() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;
		virtual void Shutdown() = 0;

		// Опциональные, можно оставить пустыми
		virtual void OnWindowMessage([[maybe_unused]] UINT msg, [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam) {}
		virtual void OnResize([[maybe_unused]] int width, [[maybe_unused]] int height) {}
	};

} // namespace ogle