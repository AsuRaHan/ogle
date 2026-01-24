// src/input/InputTypes.h
#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <string>

namespace ogle {

	// Используем Windows Virtual Keys напрямую
	using KeyCode = int;  // VK_SPACE, VK_A, VK_ESCAPE и т.д.

	// Кнопки мыши
	enum class MouseButton : uint8_t {
		Left = 0,
		Right = 1,
		Middle = 2,
		X1 = 3,
		X2 = 4,

		Count = 5
	};

	// Геймпад
	enum class GamepadButton : uint8_t {
		A = 0, B = 1, X = 2, Y = 3,
		LeftShoulder = 4, RightShoulder = 5,
		Back = 6, Start = 7,
		LeftThumb = 8, RightThumb = 9,
		DPadUp = 10, DPadRight = 11,
		DPadDown = 12, DPadLeft = 13,

		Count = 14
	};

	enum class GamepadAxis : uint8_t {
		LeftStickX = 0,
		LeftStickY = 1,
		RightStickX = 2,
		RightStickY = 3,
		LeftTrigger = 4,
		RightTrigger = 5,

		Count = 6
	};

	// Типы действий
	enum class ActionType {
		Button,
		Axis,
		Vector2,
		Vector3,
		Trigger
	};

	// Модификаторы
	struct Modifiers {
		bool ctrl : 1;
		bool shift : 1;
		bool alt : 1;
		bool win : 1;

		Modifiers() : ctrl(false), shift(false), alt(false), win(false) {}
		explicit Modifiers(int win32KeyState);
	};

} // namespace ogle