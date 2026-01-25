// src/input/InputController.cpp
#include "input/InputController.h"

namespace ogle {

	InputController& InputController::Get() {
		static InputController instance;
		return instance;
	}

	InputController::InputController() {
		RegisterDefaultAxes();
		Logger::Info("InputController initialized");
	}

	InputController::~InputController() {
		Logger::Info("InputController shutdown");
	}

	// === РЕАЛИЗАЦИЯ КЛАВИАТУРЫ ===

	bool InputController::IsKeyDown(int vk) const {
		if (vk < 0 || vk >= 256) return false;
		return m_keyStates[vk];
	}

	bool InputController::IsKeyPressed(int vk) const {
		if (vk < 0 || vk >= 256) return false;
		return m_keyPressedThisFrame[vk];
	}

	bool InputController::IsKeyReleased(int vk) const {
		if (vk < 0 || vk >= 256) return false;
		return m_keyReleasedThisFrame[vk];
	}

	// === РЕАЛИЗАЦИЯ МЫШИ ===

	glm::vec2 InputController::GetMousePosition() const {
		return m_mousePosition;
	}

	glm::vec2 InputController::GetMouseDelta() const {
		return m_mouseDelta;
	}

	float InputController::GetMouseWheelDelta() const {
		return m_mouseWheelDelta;
	}

	bool InputController::IsMouseButtonDown(int button) const {
		if (button < 0 || button >= 5) return false;
		return m_mouseButtonStates[button];
	}

	bool InputController::IsMouseButtonPressed(int button) const {
		if (button < 0 || button >= 5) return false;
		return m_mousePressedThisFrame[button];
	}

	bool InputController::IsMouseButtonReleased(int button) const {
		if (button < 0 || button >= 5) return false;
		return m_mouseReleasedThisFrame[button];
	}

	// === РЕАЛИЗАЦИЯ ГЕЙМПАДОВ ===

	bool InputController::IsGamepadConnected(int player) const {
		if (player < 0 || player >= 4) return false;
		return m_gamepads[player].connected;
	}

	bool InputController::IsGamepadButtonDown(int player, int button) const {
		if (player < 0 || player >= 4) return false;
		if (button < 0 || button >= 14) return false;
		if (!m_gamepads[player].connected) return false;
		return m_gamepads[player].buttons[button];
	}

	float InputController::GetGamepadAxis(int player, int axis) const {
		if (player < 0 || player >= 4) return 0.0f;
		if (axis < 0 || axis >= 6) return 0.0f;
		if (!m_gamepads[player].connected) return 0.0f;
		return m_gamepads[player].axes[axis];
	}

	// === INPUT ACTIONS ===

	InputAction* InputController::CreateAction(const std::string& name, ActionType type) {
		// Проверяем, есть ли уже действие с таким именем
		auto it = m_actions.find(name);
		if (it != m_actions.end()) {
			Logger::Warning("InputAction already exists: " + name);
			return it->second.get();
		}

		// Создаем новое действие
		auto action = std::make_unique<InputAction>(name, type);
		InputAction* ptr = action.get();
		m_actions[name] = std::move(action);

		Logger::Info("InputAction created: " + name);
		return ptr;
	}

	InputAction* InputController::GetAction(const std::string& name) {
		auto it = m_actions.find(name);
		if (it != m_actions.end()) {
			return it->second.get();
		}
		return nullptr;
	}

	void InputController::RemoveAction(const std::string& name) {
		auto it = m_actions.find(name);
		if (it != m_actions.end()) {
			m_actions.erase(it);
			Logger::Info("InputAction removed: " + name);
		}
	}

	bool InputController::HasAction(const std::string& name) const {
		return m_actions.find(name) != m_actions.end();
	}

	// === КОНТЕКСТЫ ===

	void InputController::PushContext(const std::string& context) {
		m_contextStack.push_back(context);
		Logger::Debug("Input context pushed: " + context);
	}

	void InputController::PopContext() {
		if (!m_contextStack.empty()) {
			std::string context = m_contextStack.back();
			m_contextStack.pop_back();
			Logger::Debug("Input context popped: " + context);
		}
	}

	const std::string& InputController::GetCurrentContext() const {
		static const std::string empty = "";
		return m_contextStack.empty() ? empty : m_contextStack.back();
	}

	// === МОДИФИКАТОРЫ ===

	const Modifiers& InputController::GetCurrentModifiers() const {
		return m_currentModifiers;
	}

	// === МЕТОДЫ ОБНОВЛЕНИЯ (для InputSystem) ===

	void InputController::UpdateKeyboardState(
		const bool keyStates[256],
		const bool keyPressed[256],
		const bool keyReleased[256]) {

		for (int i = 0; i < 256; ++i) {
			m_keyStates[i] = keyStates[i];
			m_keyPressedThisFrame[i] = keyPressed[i];
			m_keyReleasedThisFrame[i] = keyReleased[i];
		}
	}

	void InputController::UpdateMouseState(
		const glm::vec2& position,
		const glm::vec2& delta,
		float wheelDelta,
		const bool buttonStates[5],
		const bool buttonPressed[5],
		const bool buttonReleased[5]) {

		m_mousePosition = position;
		m_mouseDelta = delta;
		m_mouseWheelDelta = wheelDelta;

		for (int i = 0; i < 5; ++i) {
			m_mouseButtonStates[i] = buttonStates[i];
			m_mousePressedThisFrame[i] = buttonPressed[i];
			m_mouseReleasedThisFrame[i] = buttonReleased[i];
		}
	}

	void InputController::UpdateGamepadState(
		int player,
		bool connected,
		const bool buttons[14],
		const float axes[6]) {

		if (player < 0 || player >= 4) return;

		m_gamepads[player].connected = connected;

		if (connected) {
			for (int i = 0; i < 14; ++i) {
				m_gamepads[player].buttons[i] = buttons[i];
			}

			for (int i = 0; i < 6; ++i) {
				m_gamepads[player].axes[i] = axes[i];
			}
		}
		else {
			// Сбрасываем состояние если геймпад отключен
			for (int i = 0; i < 14; ++i) {
				m_gamepads[player].buttons[i] = false;
			}

			for (int i = 0; i < 6; ++i) {
				m_gamepads[player].axes[i] = 0.0f;
			}
		}
	}

	void InputController::UpdateModifiers(const Modifiers& modifiers) {
		m_currentModifiers = modifiers;
	}

	void InputController::UpdateActions(float deltaTime) {
		// Сначала обновляем оси
		UpdateAxes(deltaTime);

		// Затем обновляем действия (они могут использовать значения осей)
		for (auto& pair : m_actions) {
			pair.second->Update(deltaTime);
		}
	}

	void InputController::ResetFrameState() {
		// Сбрасываем временные состояния (нажато/отпущено в этом кадре)
		for (int i = 0; i < 256; ++i) {
			m_keyPressedThisFrame[i] = false;
			m_keyReleasedThisFrame[i] = false;
		}

		for (int i = 0; i < 5; ++i) {
			m_mousePressedThisFrame[i] = false;
			m_mouseReleasedThisFrame[i] = false;
		}

		// Сбрасываем дельту мыши и колесико
		m_mouseDelta = { 0.0f, 0.0f };
		m_mouseWheelDelta = 0.0f;

		// Сбрасываем состояние действий
		for (auto& pair : m_actions) {
			pair.second->ResetFrameState();
		}
	}

	// Добавляем новые методы в InputController.cpp:

	void InputController::RegisterAxis(const std::string& name,
		const std::vector<AxisBinding>& bindings) {
		if (m_axes.find(name) != m_axes.end()) {
			Logger::Warning("Axis already registered: " + name);
			return;
		}

		AxisData data;
		data.bindings = bindings;
		m_axes[name] = data;

		Logger::Info("Axis registered: " + name + " (" +
			std::to_string(bindings.size()) + " bindings)");
	}

	void InputController::RegisterAxis(const std::string& name,
		const AxisBinding& binding) {
		RegisterAxis(name, std::vector<AxisBinding>{binding});
	}

	void InputController::RegisterDefaultAxes() {
		// Horizontal: A/D + LeftStickX
		RegisterAxis("Horizontal", {
			AxisBinding::CreateKeyboardAxis('A', 'D'),
			AxisBinding::CreateGamepadAxis(0, GamepadAxis::LeftStickX)
			});

		// Vertical: W/S + LeftStickY (инвертируем)
		RegisterAxis("Vertical", {
			AxisBinding::CreateKeyboardAxis('W', 'S'),
			AxisBinding::CreateGamepadAxis(0, GamepadAxis::LeftStickY, 0.1f, -1.0f) // Инверсия
			});

		// Mouse X/Y
		RegisterAxis("MouseX",
			AxisBinding::CreateMouseAxis(0, 0.1f));
		RegisterAxis("MouseY",
			AxisBinding::CreateMouseAxis(1, -0.1f)); // Инверсия

		// Triggers
		RegisterAxis("LeftTrigger",
			AxisBinding::CreateGamepadAxis(0, GamepadAxis::LeftTrigger));
		RegisterAxis("RightTrigger",
			AxisBinding::CreateGamepadAxis(0, GamepadAxis::RightTrigger));

		Logger::Info("Default axes registered");
	}
	float InputController::GetAxis(const std::string& name) const {
		auto it = m_axes.find(name);
		if (it != m_axes.end()) {
			return it->second.value;
		}
		Logger::Warning("Axis not found: " + name);
		return 0.0f;
	}

	float InputController::GetAxisRaw(const std::string& name) const {
		auto it = m_axes.find(name);
		if (it != m_axes.end()) {
			return it->second.rawValue;
		}
		return 0.0f;
	}

	float InputController::CalculateAxisValue(const AxisData& axisData) const {
		float total = 0.0f;

		for (const auto& binding : axisData.bindings) {
			float value = 0.0f;

			switch (binding.type) {
			case AxisBinding::SourceType::KeyboardKey: {
				bool isDown = IsKeyDown(binding.keyboardKey.key);
				value = isDown ? 1.0f : 0.0f;
				value *= binding.keyboardKey.scale;
				break;
			}

			case AxisBinding::SourceType::KeyboardAxisPair: {
				float positive = IsKeyDown(binding.keyboardPair.positive) ? 1.0f : 0.0f;
				float negative = IsKeyDown(binding.keyboardPair.negative) ? -1.0f : 0.0f;
				value = positive + negative;
				break;
			}

			case AxisBinding::SourceType::MouseAxis: {
				if (binding.mouseAxis.axis == 0) {
					value = m_mouseDelta.x * binding.mouseAxis.sensitivity;
				}
				else {
					value = m_mouseDelta.y * binding.mouseAxis.sensitivity;
				}
				break;
			}

			case AxisBinding::SourceType::MouseWheel: {
				value = m_mouseWheelDelta * binding.mouseWheel.sensitivity;
				break;
			}

			case AxisBinding::SourceType::GamepadAxis: {
				value = GetGamepadAxis(binding.gamepadAxis.player,
					static_cast<int>(binding.gamepadAxis.axis));

				// Применяем deadzone
				if (std::abs(value) < binding.gamepadAxis.deadzone) {
					value = 0.0f;
				}

				value *= binding.gamepadAxis.scale;
				break;
			}
			}

			// Применяем чувствительность и инверсию
			value *= binding.sensitivity;
			if (binding.invert) {
				value = -value;
			}

			total += value;
		}

		// Ограничиваем значение
		return std::clamp(total, -1.0f, 1.0f);
	}

	void InputController::UpdateAxis(const std::string& name,
		AxisData& axisData,
		float deltaTime) {
		// Вычисляем сырое значение
		axisData.rawValue = CalculateAxisValue(axisData);

		// Если snap = true, мгновенное значение
		bool hasSnap = false;
		for (const auto& binding : axisData.bindings) {
			if (binding.snap) {
				hasSnap = true;
				break;
			}
		}

		if (hasSnap) {
			axisData.value = axisData.rawValue;
		}
		else {
			// Плавное движение к целевому значению
			float gravity = 3.0f;  // Default
			if (!axisData.bindings.empty()) {
				gravity = axisData.bindings[0].gravity;
			}

			if (std::abs(axisData.rawValue) > 0.01f) {
				// Двигаемся к целевому значению
				axisData.value = axisData.rawValue;
			}
			else {
				// Возвращаемся к 0 с заданной скоростью
				if (axisData.value > 0.0f) {
					axisData.value = std::max(0.0f, axisData.value - gravity * deltaTime);
				}
				else if (axisData.value < 0.0f) {
					axisData.value = std::min(0.0f, axisData.value + gravity * deltaTime);
				}
			}
		}
	}

	void InputController::UpdateAxes(float deltaTime) {
		for (auto& pair : m_axes) {
			UpdateAxis(pair.first, pair.second, deltaTime);
		}
	}

	void InputController::RemoveAxis(const std::string& name) {
		if (m_axes.erase(name) > 0) {
			Logger::Info("Axis removed: " + name);
		}
	}

	bool InputController::HasAxis(const std::string& name) const {
		return m_axes.find(name) != m_axes.end();
	}

	const std::vector<std::string>& InputController::GetAxisNames() const {
		static std::vector<std::string> names;
		names.clear();
		for (const auto& pair : m_axes) {
			names.push_back(pair.first);
		}
		return names;
	}



} // namespace ogle