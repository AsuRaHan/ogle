// src/input/InputAction.cpp
#include "InputAction.h"
#include <algorithm>
#include <windows.h>

namespace ogle {

	Modifiers::Modifiers(int win32KeyState) {
		ctrl = (win32KeyState & MK_CONTROL) != 0;
		shift = (win32KeyState & MK_SHIFT) != 0;
		alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
		win = (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);
	}

	InputAction::InputAction(std::string name, ActionType type)
		: m_name(std::move(name)), m_type(type) {
		m_state = ActionState{};
	}

	void InputAction::AddKey(KeyCode key, Modifiers mods, float scale) {
		Binding binding;
		binding.type = Binding::SourceType::KeyboardKey;
		binding.key = key;
		binding.modifiers = mods;
		binding.scale = scale;
		m_bindings.push_back(binding);
	}

	void InputAction::AddMouseButton(MouseButton button, Modifiers mods, float scale) {
		Binding binding;
		binding.type = Binding::SourceType::MouseButton;
		binding.mouseButton = button;
		binding.modifiers = mods;
		binding.scale = scale;
		m_bindings.push_back(binding);
	}

	void InputAction::AddGamepadButton(int player, GamepadButton button, float scale) {
		Binding binding;
		binding.type = Binding::SourceType::GamepadButton;
		binding.gamepadButton.player = static_cast<uint8_t>(player);
		binding.gamepadButton.button = button;
		binding.scale = scale;
		m_bindings.push_back(binding);
	}

	void InputAction::AddGamepadAxis(int player, GamepadAxis axis, float deadzone, float scale) {
		Binding binding;
		binding.type = Binding::SourceType::GamepadAxis;
		binding.gamepadAxis.player = static_cast<uint8_t>(player);
		binding.gamepadAxis.axis = axis;
		binding.deadzone = deadzone;
		binding.scale = scale;
		m_bindings.push_back(binding);
	}

	void InputAction::AddAxisPair(KeyCode positive, KeyCode negative, Modifiers mods) {
		// Positive key
		{
			Binding binding;
			binding.type = Binding::SourceType::KeyboardKey;
			binding.key = positive;
			binding.modifiers = mods;
			binding.scale = 1.0f;
			binding.isPositive = true;
			m_bindings.push_back(binding);
		}

		// Negative key
		{
			Binding binding;
			binding.type = Binding::SourceType::KeyboardKey;
			binding.key = negative;
			binding.modifiers = mods;
			binding.scale = -1.0f;
			binding.isPositive = false;
			m_bindings.push_back(binding);
		}
	}

	void InputAction::Update(float deltaTime) {
		// Пока пустая реализация
		// Будет работать когда интегрируем с InputSystem
	}

} // namespace ogle::input