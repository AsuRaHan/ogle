// InputConfigurator.cpp
#include "input/InputConfigurator.h"
#include <iostream>
#include "log/Logger.h"
#include "input/KeyCodes.h"

namespace ogle
{
    void InputConfigurator::ConfigureCameraControls(input::InputSystem& inputSystem, 
                                                   Camera& camera,
                                                   float& lastDeltaTime)
    {
        m_camera = &camera;
        m_lastDeltaTime = &lastDeltaTime;
        
        // Создаем контекст геймплея
        m_gameplayContext = inputSystem.CreateGameplayContext();
        if (!m_gameplayContext) 
        {
            // std::cerr << "Failed to create gameplay context!" << std::endl;
            ogle::Logger::Error("Failed to create gameplay context!");
            return;
        }
        
        // Проверяем создание осей
        if (!m_gameplayContext->GetAxis("LookHorizontal") ||
            !m_gameplayContext->GetAxis("LookVertical") ||
            !m_gameplayContext->GetAxis("MoveHorizontal") ||
            !m_gameplayContext->GetAxis("MoveVertical"))
        {
            // std::cerr << "Some input axes are missing!" << std::endl;
            ogle::Logger::Error("Some input axes are missing!");
        }
        
        // Настраиваем управление
        SetupCameraAxes();
        SetupCameraActions();
        SetupMouseCapture();
        
        ogle::Logger::Info("Camera input controls configured successfully");
    }
    
    void InputConfigurator::SetupCameraAxes()
    {
        if (!m_gameplayContext || !m_camera) return;
        
        // Ось вращения камеры (мышь)
        auto* lookHorizontal = m_gameplayContext->GetAxis("LookHorizontal");
        if (lookHorizontal)
        {
            ogle::Logger::Info("Setting up LookHorizontal axis");
            lookHorizontal->SetCallback([this](const glm::vec2& value, float deltaTime) {
                if (value.x != 0.0f)
                {
                    // Умножаем на дельта-время для плавности
                    m_camera->Rotate(value.x * 100.0f * deltaTime, 0.0f);
                }
            });
        }
        
        auto* lookVertical = m_gameplayContext->GetAxis("LookVertical");
        if (lookVertical)
        {
            ogle::Logger::Info("Setting up LookVertical axis");
            lookVertical->SetCallback([this](const glm::vec2& value, float deltaTime) {
                if (value.y != 0.0f)
                {
                    // m_camera->Rotate(0.0f, value.y * 100.0f * deltaTime);
                }
            });
        }
        
        // Оси движения камеры (WASD)
        auto* moveHorizontal = m_gameplayContext->GetAxis("MoveHorizontal");
        if (moveHorizontal)
        {
            ogle::Logger::Info("Setting up MoveHorizontal axis");
            moveHorizontal->SetCallback([this](const glm::vec2& value, float deltaTime) {
                if (value.x != 0.0f)
                {
                    float velocity = m_camera->GetMovementSpeed() * deltaTime;
                    m_camera->MoveRight(value.x * velocity);
                }
            });
        }
        
        auto* moveVertical = m_gameplayContext->GetAxis("MoveVertical");
        if (moveVertical)
        {
            ogle::Logger::Info("Setting up MoveVertical axis");
            moveVertical->SetCallback([this](const glm::vec2& value, float deltaTime) {
                if (value.y != 0.0f)
                {
                    float velocity = m_camera->GetMovementSpeed() * deltaTime;
                    m_camera->MoveForward(value.y * velocity);
                }
            });
        }
    }
    
    void InputConfigurator::SetupCameraActions()
    {
        if (!m_gameplayContext || !m_camera || !m_lastDeltaTime) return;
        
        // Движение вверх/вниз
        auto* moveUp = m_gameplayContext->CreateAction("MoveUp");
        if (moveUp)
        {
            moveUp->AddKeyBinding(ogle::input::K_Q, input::ActionTrigger::Held);
            moveUp->AddKeyBinding(ogle::input::K_SPACE, input::ActionTrigger::Held);
            moveUp->SetCallback([this](const input::InputEvent& event, float value) {
                if (event.type == input::EventType::KeyHeld)
                {
                    float velocity = m_camera->GetMovementSpeed() * (*m_lastDeltaTime);
                    m_camera->MoveUp(velocity);
                }
            });
        }
        
        auto* moveDown = m_gameplayContext->CreateAction("MoveDown");
        if (moveDown)
        {
            moveDown->AddKeyBinding(ogle::input::K_E, input::ActionTrigger::Held);
            moveDown->AddKeyBinding(ogle::input::K_CTRL, input::ActionTrigger::Held);
            moveDown->SetCallback([this](const input::InputEvent& event, float value) {
                if (event.type == input::EventType::KeyHeld)
                {
                    float velocity = m_camera->GetMovementSpeed() * (*m_lastDeltaTime);
                    m_camera->MoveUp(-velocity);
                }
            });
        }
        
        // Ускорение (спринт)
        auto* sprint = m_gameplayContext->CreateAction("Sprint");
        if (sprint)
        {
            sprint->AddKeyBinding(ogle::input::K_SHIFT, input::ActionTrigger::Pressed);
            sprint->SetCallback([this](const input::InputEvent& event, float value) {
                if (event.type == input::EventType::KeyPressed)
                {
                    m_camera->SetMovementSpeed(15.0f);
                }
                else if (event.type == input::EventType::KeyReleased)
                {
                    m_camera->SetMovementSpeed(5.0f);
                }
            });
        }
    }
    
    void InputConfigurator::SetupMouseCapture()
    {
        if (!m_gameplayContext) return;
        
        auto* toggleMouseCapture = m_gameplayContext->CreateAction("ToggleMouseCapture");
        if (toggleMouseCapture)
        {
            // Захват правой кнопкой мыши
            toggleMouseCapture->AddMouseBinding(1, input::ActionTrigger::Pressed);
            // Освобождение ESC
            toggleMouseCapture->AddKeyBinding(VK_ESCAPE, input::ActionTrigger::Pressed);
            
            toggleMouseCapture->SetCallback([](const input::InputEvent& event, float value) {
                auto& inputSystem = input::InputSystem::GetInstance();
                if (event.type == input::EventType::MouseButtonPressed)
                {
                    std::cout << "Mouse capture ON" << std::endl;
                    inputSystem.SetMouseCapture(true);
                }
                else if (event.type == input::EventType::KeyPressed)
                {
                    std::cout << "Mouse capture OFF" << std::endl;
                    inputSystem.SetMouseCapture(false);
                }
            });
        }
    }
    
    void InputConfigurator::Update(float deltaTime)
    {
        // Можно добавить обновление состояния ввода если нужно
    }
}