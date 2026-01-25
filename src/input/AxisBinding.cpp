// src/input/AxisBinding.cpp
#include "AxisBinding.h"

namespace ogle {

AxisBinding AxisBinding::CreateKeyboardKey(KeyCode key, float scale) {
    AxisBinding binding;
    binding.type = SourceType::KeyboardKey;
    binding.keyboardKey.key = key;
    binding.keyboardKey.scale = scale;
    return binding;
}

AxisBinding AxisBinding::CreateKeyboardAxis(KeyCode positive, KeyCode negative) {
    AxisBinding binding;
    binding.type = SourceType::KeyboardAxisPair;
    binding.keyboardPair.positive = positive;
    binding.keyboardPair.negative = negative;
    return binding;
}

AxisBinding AxisBinding::CreateMouseAxis(int axis, float sensitivity) {
    AxisBinding binding;
    binding.type = SourceType::MouseAxis;
    binding.mouseAxis.axis = axis;
    binding.mouseAxis.sensitivity = sensitivity;
    return binding;
}

AxisBinding AxisBinding::CreateMouseWheel(float sensitivity) {
    AxisBinding binding;
    binding.type = SourceType::MouseWheel;
    binding.mouseWheel.sensitivity = sensitivity;
    return binding;
}

AxisBinding AxisBinding::CreateGamepadAxis(int player, GamepadAxis axis, 
                                          float deadzone, float scale) {
    AxisBinding binding;
    binding.type = SourceType::GamepadAxis;
    binding.gamepadAxis.player = player;
    binding.gamepadAxis.axis = axis;
    binding.gamepadAxis.deadzone = deadzone;
    binding.gamepadAxis.scale = scale;
    return binding;
}

} // namespace ogle