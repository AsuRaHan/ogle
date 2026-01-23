// InputSystem.cpp
#include "InputSystem.h"
#include <iostream>

namespace ogle::input
{
    InputSystem::InputSystem() = default;
    
    InputSystem::~InputSystem()
    {
        Shutdown();
    }
    
    InputSystem& InputSystem::GetInstance()
    {
        static InputSystem instance;
        return instance;
    }
    
    void InputSystem::Initialize(HWND hwnd)
    {
        if (m_initialized) return;
        
        m_hwnd = hwnd;
        m_keys.fill(false);  // Используем fill для array
        m_prevKeys.fill(false);
        
        // Инициализируем начальную позицию мыши
        if (m_hwnd)
        {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(m_hwnd, &pt);
            m_mousePos.x = static_cast<float>(pt.x);
            m_mousePos.y = static_cast<float>(pt.y);
            m_prevMousePos = m_mousePos;
        }
        
        m_initialized = true;
        std::cout << "InputSystem initialized" << std::endl;
    }
    
    void InputSystem::Shutdown()
    {
        if (!m_initialized) return;
        
        if (m_mouseCaptured)
        {
            SetMouseCapture(false);
        }
        
        m_contexts.clear();
        m_initialized = false;
    }
    
    void InputSystem::Update(float deltaTime)
    {
        if (!m_initialized) return;
        
        // Обновляем предыдущие состояния
        m_prevKeys = m_keys;  // Копируем весь array
        for (int i = 0; i < 3; ++i)
        {
            m_prevMouseButtons[i] = m_mouseButtons[i];
        }
        
        m_prevMousePos = m_mousePos;
        
        // Сбрасываем дельту мыши
        m_mouseDelta = glm::vec2(0.0f);
        m_mouseWheelDelta = 0.0f;
        
        // Обновляем захват мыши
        if (m_mouseCaptured && m_hwnd)
        {
            RECT rect;
            GetClientRect(m_hwnd, &rect);
            
            float centerX = (rect.right - rect.left) / 2.0f;
            float centerY = (rect.bottom - rect.top) / 2.0f;
            
            // Центрируем курсор
            POINT center = { 
                static_cast<LONG>(centerX), 
                static_cast<LONG>(centerY) 
            };
            ClientToScreen(m_hwnd, &center);
            SetCursorPos(center.x, center.y);
            
            // Обновляем позицию мыши
            m_mousePos.x = centerX;
            m_mousePos.y = centerY;
        }
        
        // Обрабатываем очередь событий
        for (auto& event : m_eventQueue)
        {
            for (auto& [name, context] : m_contexts)
            {
                if (context->IsActive())
                {
                    context->ProcessEvent(*event, deltaTime);
                }
            }
        }
        m_eventQueue.clear();
        
        // Обновляем контексты
        for (auto& [name, context] : m_contexts)
        {
            if (context->IsActive())
            {
                context->Update(deltaTime);
            }
        }
    }
    
    void InputSystem::ProcessWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (!m_initialized) return;
        
        switch (msg)
        {
        case WM_KEYDOWN:
            ProcessKeyEvent(static_cast<int>(wParam), true);
            break;
            
        case WM_KEYUP:
            ProcessKeyEvent(static_cast<int>(wParam), false);
            break;
            
        case WM_MOUSEMOVE:
        {
            float x = static_cast<float>(LOWORD(lParam));
            float y = static_cast<float>(HIWORD(lParam));
            ProcessMouseMove(x, y);
            break;
        }
            
        case WM_LBUTTONDOWN:
            ProcessMouseButtonEvent(0, true);
            break;
        case WM_LBUTTONUP:
            ProcessMouseButtonEvent(0, false);
            break;
        case WM_RBUTTONDOWN:
            ProcessMouseButtonEvent(1, true);
            break;
        case WM_RBUTTONUP:
            ProcessMouseButtonEvent(1, false);
            break;
        case WM_MBUTTONDOWN:
            ProcessMouseButtonEvent(2, true);
            break;
        case WM_MBUTTONUP:
            ProcessMouseButtonEvent(2, false);
            break;
            
        case WM_MOUSEWHEEL:
        {
            float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
            ProcessMouseScroll(delta);
            break;
        }
            
        case WM_KILLFOCUS:
            // При потере фокуса сбрасываем все состояния
            m_keys.fill(false);  // Используем fill
            for (int i = 0; i < 3; ++i)
            {
                m_mouseButtons[i] = false;
            }
            if (m_mouseCaptured)
            {
                SetMouseCapture(false);
            }
            break;
        }
    }
    
    void InputSystem::ProcessKeyEvent(int keyCode, bool pressed)
    {
        if (keyCode < 0 || keyCode >= 256) return;
        
        m_keys[keyCode] = pressed;  // Правильный доступ к элементу array
        
        // Создаем событие клавиши
        auto event = std::make_unique<KeyEvent>();
        event->type = pressed ? EventType::KeyPressed : EventType::KeyReleased;
        event->keyCode = keyCode;
        // TODO: Добавить shift/ctrl/alt состояния
        m_eventQueue.push_back(std::move(event));
    }
    
    void InputSystem::ProcessMouseButtonEvent(int button, bool pressed)
    {
        if (button < 0 || button >= 3) return;
        
        m_mouseButtons[button] = pressed;
        
        // Создаем событие кнопки мыши
        auto event = std::make_unique<MouseButtonEvent>();
        event->type = pressed ? EventType::MouseButtonPressed : EventType::MouseButtonReleased;
        event->button = button;
        event->position = m_mousePos;
        m_eventQueue.push_back(std::move(event));
    }
    
    void InputSystem::ProcessMouseScroll(float delta)
    {
        m_mouseWheelDelta = delta;
        
        // Создаем событие прокрутки мыши
        auto event = std::make_unique<MouseScrollEvent>();
        event->type = EventType::MouseScrolled;
        event->offset = glm::vec2(0.0f, delta);
        m_eventQueue.push_back(std::move(event));
    }
    
    void InputSystem::ProcessMouseMove(float x, float y)
    {
        if (m_mouseCaptured)
        {
            // Вычисляем дельту от центра окна
            RECT rect;
            GetClientRect(m_hwnd, &rect);
            
            float centerX = (rect.right - rect.left) / 2.0f;
            float centerY = (rect.bottom - rect.top) / 2.0f;
            
            m_mouseDelta.x = x - centerX;
            m_mouseDelta.y = centerY - y;  // Инвертируем Y
            
            // Создаем событие движения мыши
            auto event = std::make_unique<MouseMoveEvent>();
            event->type = EventType::MouseMoved;
            event->position = glm::vec2(centerX, centerY);
            event->delta = m_mouseDelta;
            m_eventQueue.push_back(std::move(event));
        }
        else
        {
            m_mousePos.x = x;
            m_mousePos.y = y;
            
            // Создаем событие движения мыши
            auto event = std::make_unique<MouseMoveEvent>();
            event->type = EventType::MouseMoved;
            event->position = m_mousePos;
            event->delta = m_mousePos - m_prevMousePos;
            m_eventQueue.push_back(std::move(event));
        }
    }
    
    InputContext* InputSystem::CreateGameplayContext()
    {
        auto* context = CreateContext("Gameplay");
        if (!context) return nullptr;
        
        // Создаем оси для плавного движения
        auto* moveHorizontal = context->CreateAxis("MoveHorizontal");
        if (moveHorizontal)
        {
            moveHorizontal->AddKeyBinding('A', 'D');
        }
        
        auto* moveVertical = context->CreateAxis("MoveVertical");
        if (moveVertical)
        {
            moveVertical->AddKeyBinding('S', 'W');
        }
        
        auto* lookHorizontal = context->CreateAxis("LookHorizontal");
        if (lookHorizontal)
        {
            lookHorizontal->AddMouseAxisBinding(true, 1.0f);  // X-ось мыши
        }
        
        auto* lookVertical = context->CreateAxis("LookVertical");
        if (lookVertical)
        {
            lookVertical->AddMouseAxisBinding(false, 1.0f);  // Y-ось мыши
        }
        
        return context;
    }
    
    InputContext* InputSystem::CreateContext(const std::string& name)
    {
        if (m_contexts.find(name) != m_contexts.end())
        {
            return m_contexts[name].get();
        }
        
        auto context = std::make_unique<InputContext>(name);
        auto* ptr = context.get();
        m_contexts[name] = std::move(context);
        return ptr;
    }
    
    InputContext* InputSystem::GetContext(const std::string& name)
    {
        auto it = m_contexts.find(name);
        if (it != m_contexts.end())
        {
            return it->second.get();
        }
        return nullptr;
    }
    
    bool InputSystem::IsKeyDown(int keyCode) const
    {
        if (keyCode >= 0 && keyCode < 256)
            return m_keys[keyCode];
        return false;
    }
    
    bool InputSystem::IsKeyPressed(int keyCode) const
    {
        if (keyCode >= 0 && keyCode < 256)
            return m_keys[keyCode] && !m_prevKeys[keyCode];
        return false;
    }
    
    bool InputSystem::IsKeyReleased(int keyCode) const
    {
        if (keyCode >= 0 && keyCode < 256)
            return !m_keys[keyCode] && m_prevKeys[keyCode];
        return false;
    }
    
    bool InputSystem::IsMouseButtonDown(int button) const
    {
        if (button >= 0 && button < 3)
            return m_mouseButtons[button];
        return false;
    }
    
    glm::vec2 InputSystem::GetMousePosition() const
    {
        return m_mousePos;
    }
    
    glm::vec2 InputSystem::GetMouseDelta() const
    {
        return m_mouseDelta;
    }
    
    float InputSystem::GetMouseWheelDelta() const
    {
        return m_mouseWheelDelta;
    }
    
    void InputSystem::SetMouseCapture(bool capture)
    {
        if (!m_hwnd || m_mouseCaptured == capture) return;
        
        m_mouseCaptured = capture;
        
        if (capture)
        {
            ShowCursor(FALSE);
            
            RECT rect;
            GetClientRect(m_hwnd, &rect);
            
            POINT topLeft = { rect.left, rect.top };
            POINT bottomRight = { rect.right, rect.bottom };
            
            ClientToScreen(m_hwnd, &topLeft);
            ClientToScreen(m_hwnd, &bottomRight);
            
            rect.left = topLeft.x;
            rect.top = topLeft.y;
            rect.right = bottomRight.x;
            rect.bottom = bottomRight.y;
            
            ClipCursor(&rect);
        }
        else
        {
            ShowCursor(TRUE);
            ClipCursor(nullptr);
        }
    }
}