// src/input/AxisBinding.h
#pragma once

#include "InputTypes.h"
#include <vector>
#include <string>

namespace ogle {

// Одно привязка к оси
struct AxisBinding {
    enum class SourceType {
        KeyboardKey,      // Одна клавиша (A/D)
        KeyboardAxisPair, // Пара клавиш (A/D, W/S)
        MouseAxis,        // Ось мыши (X/Y)
        MouseWheel,       // Колесико мыши
        GamepadAxis,      // Ось геймпада
        GamepadDPad,      // D-Pad как ось
        GamepadTrigger    // Триггер как ось
    };
    
    SourceType type;
    
    union {
        // Для KeyboardKey
        struct {
            KeyCode key;
            float scale;  // Обычно 1.0 или -1.0
        } keyboardKey;
        
        // Для KeyboardAxisPair
        struct {
            KeyCode positive;
            KeyCode negative;
        } keyboardPair;
        
        // Для MouseAxis
        struct {
            int axis;  // 0 = X, 1 = Y
            float sensitivity;
        } mouseAxis;
        
        // Для MouseWheel
        struct {
            float sensitivity;
        } mouseWheel;
        
        // Для GamepadAxis
        struct {
            int player;
            GamepadAxis axis;
            float deadzone;
            float scale;
        } gamepadAxis;
    };
    
    // Общие параметры
    float gravity = 3.0f;      // Скорость возврата к 0
    float sensitivity = 1.0f;  // Чувствительность
    bool invert = false;       // Инверсия
    bool snap = false;         // Мгновенное переключение (для цифрового ввода)
    
    AxisBinding() = default;
    
    // Конструкторы для удобства
    static AxisBinding CreateKeyboardKey(KeyCode key, float scale = 1.0f);
    static AxisBinding CreateKeyboardAxis(KeyCode positive, KeyCode negative);
    static AxisBinding CreateMouseAxis(int axis, float sensitivity = 1.0f);
    static AxisBinding CreateMouseWheel(float sensitivity = 1.0f);
    static AxisBinding CreateGamepadAxis(int player, GamepadAxis axis, 
                                         float deadzone = 0.1f, float scale = 1.0f);
};

} // namespace ogle