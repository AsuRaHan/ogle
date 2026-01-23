// InputSystem.h
#pragma once
#include <windows.h>
#include <glm/glm.hpp>
#include <functional>
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>
#include "InputContext.h"
#include "InputEvents.h"
#include "KeyboardState.h"

namespace ogle::input
{
    // Главная система ввода
    class InputSystem
    {
    public:
        static InputSystem& GetInstance();
        
        // Инициализация/очистка
        void Initialize(HWND hwnd);
        void Shutdown();
        
        // Обновление
        void Update(float deltaTime);
        
        // Обработка сообщений Windows
        void ProcessWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam);
        
        // Управление контекстами
        InputContext* CreateContext(const std::string& name);
        InputContext* GetContext(const std::string& name);
        void RemoveContext(const std::string& name);
        void SetContextActive(const std::string& name, bool active);
        
        // Глобальные состояния
        bool IsKeyDown(int keyCode) const;
        bool IsKeyPressed(int keyCode) const;
        bool IsKeyReleased(int keyCode) const;
        
        bool IsMouseButtonDown(int button) const;
        glm::vec2 GetMousePosition() const;
        glm::vec2 GetMouseDelta() const;
        float GetMouseWheelDelta() const;
        
        // Захват мыши
        void SetMouseCapture(bool capture);
        bool IsMouseCaptured() const { return m_mouseCaptured; }
        
        // Создание предустановленных контекстов
        InputContext* CreateGameplayContext();
        InputContext* CreateUIContext();
        InputContext* CreateMenuContext();
        
    
        void ProcessKeyEvent(int keyCode, bool pressed);
        void ProcessMouseButtonEvent(int button, bool pressed);
        void ProcessMouseScroll(float delta);
        void ProcessMouseMove(float x, float y);
    private:
        InputSystem();
        ~InputSystem();
        
        // Обновление состояний мыши/клавиатуры
        void UpdateStates(float deltaTime);
        
        // HWND окна
        HWND m_hwnd = nullptr;
        
        // Низкоуровневые состояния
        // keyboard state
        std::unique_ptr<KeyboardState> m_keyboardState;
        
        glm::vec2 m_mousePos{};
        glm::vec2 m_prevMousePos{};
        glm::vec2 m_mouseDelta{};
        
        bool m_mouseButtons[3] = { false, false, false };
        bool m_prevMouseButtons[3] = { false, false, false };
        
        float m_mouseWheelDelta = 0.0f;
        
        bool m_mouseCaptured = false;
        bool m_initialized = false;
        
        // Контексты
        std::unordered_map<std::string, std::unique_ptr<InputContext>> m_contexts;
        
        // Очередь событий
        std::vector<std::unique_ptr<InputEvent>> m_eventQueue;
    };
}