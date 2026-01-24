// src/systems/InputSystem.h
#pragma once

#include <windows.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>
#include <vector>

#include "core/ISystem.h"
#include "input/InputTypes.h"
#include "managers/InputManager.h"

// Предварительное объявление
namespace ogle{
	class InputAction;
	enum class ActionType;
}

namespace ogle {

	class InputSystem final : public ISystem {
	public:
		InputSystem();
		~InputSystem() override;

		const std::string& GetName() const override;
		bool Initialize() override;
		void Update(float deltaTime) override;
		void Render() override {}
		void Shutdown() override;
		void OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

		// Window/Device
		void SetWindowHandle(HWND hwnd);

		// Keyboard
		bool IsKeyDown(int vk) const;
		bool IsKeyPressed(int vk) const;
		bool IsKeyReleased(int vk) const;

		// Mouse
		glm::vec2 GetMousePosition() const;
		glm::vec2 GetMouseDelta() const;
		float GetMouseWheelDelta() const;
		bool IsMouseButtonDown(int button) const;
		bool IsMouseButtonPressed(int button) const;
		bool IsMouseButtonReleased(int button) const;

		// Gamepad
		bool IsGamepadConnected(int player = 0) const;
		bool IsGamepadButtonDown(int player, int button) const;
		float GetGamepadAxis(int player, int axis) const;

		// Mouse capture
		void SetMouseCapture(bool capture);
		bool IsMouseCaptured() const { return m_mouseCaptured; }

		// Input Actions
		InputAction* CreateAction(const std::string& name, ActionType type);
		InputAction* GetAction(const std::string& name);
		void RemoveAction(const std::string& name);

		// Contexts
		void PushContext(const std::string& context);
		void PopContext();
		const std::string& GetCurrentContext() const;

	private:
		// Message processing
		void ProcessKeyEvent(UINT msg, WPARAM wParam, LPARAM lParam);
		void ProcessMouseEvent(UINT msg, WPARAM wParam, LPARAM lParam);
		void ProcessRawInput(LPARAM lParam);

		// Update
		void UpdateGamepads();
		void UpdateActions(float deltaTime);
		void ResetFrameState();

		// Helper functions (добавляем объявления)
		static float NormalizeAxis(SHORT value);
		static void ApplyDeadzone(float& x, float& y, SHORT deadzone);

		// State
		HWND m_hwnd = nullptr;

		// Keyboard state
		bool m_keyStates[256] = { false };
		bool m_keyPressedThisFrame[256] = { false };
		bool m_keyReleasedThisFrame[256] = { false };

		// Mouse state
		glm::vec2 m_mousePosition = { 0.0f, 0.0f };
		glm::vec2 m_mouseDelta = { 0.0f, 0.0f };
		float m_mouseWheelDelta = 0.0f;
		bool m_mouseButtonStates[5] = { false };
		bool m_mousePressedThisFrame[5] = { false };
		bool m_mouseReleasedThisFrame[5] = { false };

		// Gamepad state
		struct GamepadState {
			bool connected = false;
			bool buttons[14] = { false };
			float axes[6] = { 0.0f };

			void Reset() {
				connected = false;
				memset(buttons, 0, sizeof(buttons));
				memset(axes, 0, sizeof(axes));
			}
		};
		GamepadState m_gamepads[4];

		// Actions
		std::unordered_map<std::string, std::unique_ptr<InputAction>> m_actions;
		std::vector<std::string> m_contextStack;

		// Mouse capture
		bool m_mouseCaptured = false;
		RECT m_clipRect = {};
		POINT m_lastMousePos = {};

		// Modifiers
		Modifiers m_currentModifiers;
	};

} // namespace ogle