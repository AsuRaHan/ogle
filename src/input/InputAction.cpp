// src/input/InputAction.cpp
#include "InputAction.h"
#include "InputController.h"  // Для доступа к состоянию ввода



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
        // Сохраняем старое состояние
        bool wasActive = m_state.active;
        float oldValue = m_state.value;

        // Получаем контроллер
        auto& controller = InputController::Get();

        // Обновляем состояние из контроллера
        UpdateFromController(&controller);

        // Обновляем время удержания
        if (m_state.active) {
            m_state.holdTime += deltaTime;
            m_state.held = true;
        }
        else {
            m_state.holdTime = 0.0f;
            m_state.held = false;
        }

        // Определяем события нажатия/отпускания
        if (m_state.active && !wasActive) {
            m_state.pressed = true;
            if (m_onPressed) {
                m_onPressed(m_state);
            }
        }
        else if (!m_state.active && wasActive) {
            m_state.released = true;
            if (m_onReleased) {
                m_onReleased(m_state);
            }
        }

        // Вызов колбэка удержания
        if (m_state.active && m_state.holdTime > 0.0f && m_onHeld) {
            m_onHeld(m_state);
        }
    }

    void InputAction::UpdateFromController(const InputController* controller) {
        if (!controller) return;

        float totalValue = 0.0f;
        bool anyActive = false;

        // Оцениваем все биндинги
        for (const auto& binding : m_bindings) {
            float value = EvaluateBinding(binding, controller);

            if (binding.type == Binding::SourceType::KeyboardKey ||
                binding.type == Binding::SourceType::MouseButton ||
                binding.type == Binding::SourceType::GamepadButton) {
                // Для кнопок: любое нажатие активирует действие
                if (std::abs(value) > 0.5f) {  // Порог для кнопок
                    anyActive = true;
                    totalValue = std::max(totalValue, value);
                }
            }
            else {
                // Для осей: суммируем значения
                totalValue += value;
                if (std::abs(value) > binding.deadzone) {
                    anyActive = true;
                }
            }
        }

        // Обновляем состояние
        m_state.active = anyActive;
        m_state.value = totalValue;

        // Для Vector2/Vector3 действий нужно будет расширить логику
        if (m_type == ActionType::Axis) {
            m_state.value = std::clamp(totalValue, -1.0f, 1.0f);
        }
    }

    float InputAction::Evaluate() const {
        auto& controller = InputController::Get();
        float totalValue = 0.0f;

        for (const auto& binding : m_bindings) {
            totalValue += EvaluateBinding(binding, &controller);
        }

        return totalValue;
    }

    float InputAction::EvaluateBinding(const Binding& binding, const InputController* controller) const {
        if (!controller) return 0.0f;

        // Проверяем модификаторы
        if (!CheckModifiers(binding.modifiers, controller->GetCurrentModifiers())) {
            return 0.0f;
        }

        float value = 0.0f;

        switch (binding.type) {
        case Binding::SourceType::KeyboardKey: {
            bool isDown = controller->IsKeyDown(binding.key);
            value = isDown ? 1.0f : 0.0f;
            break;
        }

        case Binding::SourceType::MouseButton: {
            bool isDown = controller->IsMouseButtonDown(static_cast<int>(binding.mouseButton));
            value = isDown ? 1.0f : 0.0f;
            break;
        }

        case Binding::SourceType::GamepadButton: {
            bool isDown = controller->IsGamepadButtonDown(
                binding.gamepadButton.player,
                static_cast<int>(binding.gamepadButton.button)
            );
            value = isDown ? 1.0f : 0.0f;
            break;
        }

        case Binding::SourceType::GamepadAxis: {
            value = controller->GetGamepadAxis(
                binding.gamepadAxis.player,
                static_cast<int>(binding.gamepadAxis.axis)
            );

            // Применяем deadzone
            if (std::abs(value) < binding.deadzone) {
                value = 0.0f;
            }
            break;
        }
        }

        // Применяем масштаб и инверсию
        value *= binding.scale;
        if (binding.invert) {
            value = -value;
        }

        return value;
    }

    bool InputAction::CheckModifiers(const Modifiers& required, const Modifiers& current) const {
        // Все требуемые модификаторы должны быть нажаты
        if (required.ctrl && !current.ctrl) return false;
        if (required.shift && !current.shift) return false;
        if (required.alt && !current.alt) return false;
        if (required.win && !current.win) return false;

        // Дополнительные модификаторы не должны быть нажаты (опционально)
        // Можно добавить флаг "exact match" если нужно

        return true;
    }

} // namespace ogle::input