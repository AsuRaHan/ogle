// src/input/InputAction.h
#pragma once

#include <algorithm>
#include <windows.h>
#include <functional>
#include <vector>
#include <string>
#include <memory>

//#include "InputController.h"  // Для доступа к состоянию ввода
#include "InputTypes.h"

namespace ogle {

	class InputController;

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
		//void Update(float deltaTime);
		//void ResetFrameState() { m_state.ResetFrame(); }

		//// Геттеры
		//const std::string& GetName() const { return m_name; }
		//ActionType GetType() const { return m_type; }
		//const ActionState& GetState() const { return m_state; }

		//// Коллбэки
		//using Callback = std::function<void(const ActionState&)>;
		//void OnPressed(Callback cb) { m_onPressed = std::move(cb); }
		//void OnReleased(Callback cb) { m_onReleased = std::move(cb); }
		//void OnHeld(Callback cb) { m_onHeld = std::move(cb); }

		// Обновление (будет вызываться InputController)
		void Update(float deltaTime);
		void ResetFrameState() { m_state.ResetFrame(); }

		// Проверка биндингов (публичная для отладки)
		float Evaluate() const;

		// Геттеры
		const std::string& GetName() const { return m_name; }
		ActionType GetType() const { return m_type; }
		const ActionState& GetState() const { return m_state; }

		// Колбэки
		using Callback = std::function<void(const ActionState&)>;
		void OnPressed(Callback cb) { m_onPressed = std::move(cb); }
		void OnReleased(Callback cb) { m_onReleased = std::move(cb); }
		void OnHeld(Callback cb) { m_onHeld = std::move(cb); }

		// Для InputController (дружественный доступ к состоянию)
		void UpdateFromController(const InputController* controller);

	private:
		friend class InputController;  // Даем доступ контроллеру

		std::string m_name;
		ActionType m_type;
		std::vector<Binding> m_bindings;
		ActionState m_state;

		Callback m_onPressed;
		Callback m_onReleased;
		Callback m_onHeld;

		// Вспомогательные методы для оценки биндингов
		float EvaluateBinding(const Binding& binding, const InputController* controller) const;
		bool CheckModifiers(const Modifiers& required, const Modifiers& current) const;

		friend class InputSystem;
	};

} // namespace ogle::input