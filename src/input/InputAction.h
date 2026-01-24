// src/input/InputAction.h
#pragma once

#include "InputTypes.h"
#include <functional>
#include <vector>
#include <string>
#include <memory>

namespace ogle {

	// Состояние действия
	struct ActionState {
		bool active = false;
		float value = 0.0f;
		glm::vec2 vector2 = { 0.0f, 0.0f };
		glm::vec3 vector3 = { 0.0f, 0.0f, 0.0f };

		bool pressed = false;
		bool released = false;
		bool held = false;
		float holdTime = 0.0f;

		void ResetFrame() {
			pressed = false;
			released = false;
		}
	};

	// Один биндинг
	struct Binding {
		enum class SourceType {
			KeyboardKey,
			MouseButton,
			GamepadButton,
			GamepadAxis
		};

		SourceType type;

		union {
			KeyCode key;
			MouseButton mouseButton;
			struct {
				uint8_t player;
				GamepadButton button;
			} gamepadButton;
			struct {
				uint8_t player;
				GamepadAxis axis;
			} gamepadAxis;
		};

		Modifiers modifiers;
		float scale = 1.0f;
		float deadzone = 0.0f;
		bool invert = false;
		bool isPositive = true;
	};

	// Input Action
	class InputAction {
	public:
		InputAction(std::string name, ActionType type);

		// Биндинги
		void AddKey(KeyCode key, Modifiers mods = {}, float scale = 1.0f);
		void AddMouseButton(MouseButton button, Modifiers mods = {}, float scale = 1.0f);
		void AddGamepadButton(int player, GamepadButton button, float scale = 1.0f);
		void AddGamepadAxis(int player, GamepadAxis axis, float deadzone = 0.1f, float scale = 1.0f);

		// Для составных осей
		void AddAxisPair(KeyCode positive, KeyCode negative, Modifiers mods = {});

		// Обновление
		void Update(float deltaTime);
		void ResetFrameState() { m_state.ResetFrame(); }

		// Геттеры
		const std::string& GetName() const { return m_name; }
		ActionType GetType() const { return m_type; }
		const ActionState& GetState() const { return m_state; }

		// Коллбэки
		using Callback = std::function<void(const ActionState&)>;
		void OnPressed(Callback cb) { m_onPressed = std::move(cb); }
		void OnReleased(Callback cb) { m_onReleased = std::move(cb); }
		void OnHeld(Callback cb) { m_onHeld = std::move(cb); }

	private:
		friend class InputSystem;

		std::string m_name;
		ActionType m_type;
		std::vector<Binding> m_bindings;
		ActionState m_state;

		Callback m_onPressed;
		Callback m_onReleased;
		Callback m_onHeld;
	};

} // namespace ogle::input