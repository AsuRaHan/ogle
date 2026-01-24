// InputConfigurator.h
#pragma once

#include "input/InputSystem.h"
#include "render/Camera.h"
#include <memory>

namespace ogle
{
    class InputConfigurator
    {
    public:
        InputConfigurator() = default;
        
        // Конфигурация ввода для камеры
        void ConfigureCameraControls(input::InputSystem& inputSystem, 
                                    Camera& camera,
                                    float& lastDeltaTime);
        
        // Получить созданный контекст геймплея
        input::InputContext* GetGameplayContext() const { return m_gameplayContext; }
        
        // Обновление (если нужно)
        void Update(float deltaTime);
        
    private:
        input::InputContext* m_gameplayContext = nullptr;
        Camera* m_camera = nullptr;
        float* m_lastDeltaTime = nullptr;
        
        // Вспомогательные методы
        void SetupCameraAxes();
        void SetupCameraActions();
        void SetupMouseCapture();
    };
}