// src/test/TestCubeUI.cpp
#include "TestCubeUI.h"
#include "test/TestCube.h"
#include "ui/UIController.h"
#include "log/Logger.h"
#include <string>

namespace ogle {

TestCubeUI::TestCubeUI(TestCube* cube) : m_cube(cube) {
    Logger::Info("TestCubeUI created");
}

void TestCubeUI::Initialize() {
    Logger::Info("TestCubeUI initializing...");
    
    auto& ui = UIController::Get();
    
    // Регистрируем одно окно
    ui.RegisterWindow("test_window", "Test Cube UI");
    
    // Кнопка 1
    auto* button1 = ui.CreateButton("btn1", "Button 1", {120, 30});
    button1->onClick = [this]() {
        m_button1Clicks++;
        Logger::Info("Button 1 clicked: " + std::to_string(m_button1Clicks));
    };
    ui.AddToWindow("test_window", "btn1");
    
    // Кнопка 2  
    auto* button2 = ui.CreateButton("btn2", "Button 2", {120, 30});
    button2->onClick = [this]() {
        m_button2Clicks++;
        Logger::Info("Button 2 clicked: " + std::to_string(m_button2Clicks));
    };
    ui.AddToWindow("test_window", "btn2");
    
    // Текстовое поле (будем менять)
    auto* textLabel = ui.CreateLabel("text_label", "Click buttons to change me!");
    ui.AddToWindow("test_window", "text_label");
    
    Logger::Info("TestCubeUI initialized");
}

void TestCubeUI::Update(float deltaTime) {
    m_time += deltaTime;
    
    auto& ui = UIController::Get();
    
    // Получаем текстовую метку
    if (auto* label = ui.GetElementAs<UILabel>("text_label")) {
        // Меняем текст в зависимости от нажатых кнопок
        if (m_button1Clicks > 0 && m_button2Clicks > 0) {
            label->text = "Both buttons clicked! 1:" + 
                         std::to_string(m_button1Clicks) + 
                         " 2:" + std::to_string(m_button2Clicks);
        }
        else if (m_button1Clicks > 0) {
            label->text = "Button 1 clicked " + 
                         std::to_string(m_button1Clicks) + " times";
        }
        else if (m_button2Clicks > 0) {
            label->text = "Button 2 clicked " + 
                         std::to_string(m_button2Clicks) + " times";
        }
        else {
            label->text = "Click buttons to change me!";
        }
    }
    
    // Простое логирование каждые 5 секунд
    static float logTimer = 0.0f;
    logTimer += deltaTime;
    if (logTimer >= 5.0f) {
        Logger::Info("UI Stats - Button1: " + std::to_string(m_button1Clicks) +
                    ", Button2: " + std::to_string(m_button2Clicks));
        logTimer = 0.0f;
    }
}

} // namespace ogle