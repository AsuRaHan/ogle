#include "core/ExampleLayer.h"
#include "App.h"
#include "Logger.h"
#include "imgui.h"

// Включите заголовки для менеджеров, с которыми этот слой взаимодействует
#include "managers/CameraManager.h"
#include "managers/WorldManager.h"

ExampleLayer::ExampleLayer()
    : Layer("ExampleLayer") // Передаем имя для отладки в базовый класс Layer
{
    // Лучше избегать тяжелой работы в конструкторе.
    // Инициализацию следует проводить в OnAttach().
}

ExampleLayer::~ExampleLayer()
{
    // Здесь можно очистить ресурсы, если это необходимо.
    // OnDetach() - основное место для очистки.
}

// Этот метод вызывается один раз при добавлении слоя в LayerStack.
// Это идеальное место для инициализации, загрузки ресурсов и подписки на события.
void ExampleLayer::OnAttach()
{
    LOG_INFO("ExampleLayer::OnAttach() called.");

    // --- Доступ к системам приложения ---
    // Главный экземпляр App содержит все основные менеджеры и системы.
    // Мы можем получить их через синглтон App.
    m_cameraManager = &App::Get()->GetCameraManager();
    m_worldManager = &App::Get()->GetWorldManager();

    // Пример: здесь вы можете загружать ассеты, специфичные для этого слоя.
    // auto& resourceManager = App::Get()->GetResourceManager();
    // m_myTexture = resourceManager.LoadTexture("path/to/texture.png");
}

// Этот метод вызывается один раз при удалении слоя из LayerStack.
// Это идеальное место для очистки, освобождения ресурсов и отписки от событий.
void ExampleLayer::OnDetach()
{
    LOG_INFO("ExampleLayer::OnDetach() called.");
    // Пример: освобождение ресурсов
    // m_myTexture = nullptr;
}

// Этот метод вызывается каждый кадр.
// Вся не-рендеринговая логика должна находиться здесь (например, физика, ИИ, обработка ввода игрока).
void ExampleLayer::OnUpdate(float deltaTime)
{
    // 'deltaTime' - это время, прошедшее с предыдущего кадра, в секундах.
    // Это крайне важно для независимых от частоты кадров движений и обновлений.

    // Пример: обновление анимации или значения с течением времени.
    m_exampleValue += deltaTime;
    if (m_exampleValue > 5.0f) {
        // LOG_TRACE("ExampleLayer: 5 seconds have passed.");
        m_exampleValue = 0.0f;
    }

    // Пример: взаимодействие с менеджером.
    // if (m_cameraManager) {
    //     // m_cameraManager->GetActiveCamera().Rotate({0, 1.0f * deltaTime, 0});
    // }
}

// Этот метод вызывается каждый кадр в блоке рендеринга ImGui.
// Весь UI-код ImGui для этого слоя должен быть размещен здесь.
void ExampleLayer::OnImGuiRender()
{
    // Всегда хорошая практика оборачивать окна ImGui в пары Begin/End.
    ImGui::Begin("Example Layer Control Panel");

    // Отображение некоторой информации
    ImGui::Text("This is the ExampleLayer's custom UI window.");
    ImGui::Text("It demonstrates how to create UI for a specific layer.");
    ImGui::Separator();

    // Пример простого элемента управления
    ImGui::Checkbox("Show ImGui Demo Window", &m_showDemoWindow);
    ImGui::SliderFloat("Example Value", &m_exampleValue, 0.0f, 10.0f);

    if (ImGui::Button("Log a Warning"))
    {
        LOG_WARN("This is a sample warning from ExampleLayer.");
    }

    ImGui::End();

    // Вы также можете показывать другие окна, например, демонстрационное окно ImGui.
    if (m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
}
