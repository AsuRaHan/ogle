#include "Engine.h"
#include "window/MainWindow.h"
#include <iostream>

Engine::Engine(HINSTANCE hInstance) : m_hInstance(hInstance) {}

Engine::~Engine()
{
    Shutdown();
}

bool Engine::Initialize()
{
    // 1. Окно
    m_window = std::make_unique<MainWindow>(m_hInstance);
    if (!m_window->CreateMainWindow(L"OGLE 3D Engine", 1280, 720))
    {
        std::cerr << "Не удалось создать окно" << std::endl;
        return false;
    }
    
    // Инициализируем новую систему ввода
    auto& inputSystem = ogle::input::InputSystem::GetInstance();
    inputSystem.Initialize(m_window->GetHWND());
    
    m_window->SetMessageCallback([this](UINT msg, WPARAM wParam, LPARAM lParam) {
        this->HandleWindowMessage(msg, wParam, lParam);
    });

    m_window->SetResizeCallback([this](int w, int h) {
        m_aspectRatio = static_cast<float>(w) / h;
        std::cout << "Окно ресайзнуто: новый aspect = " << m_aspectRatio << std::endl;
    });

    // Инициализируем изначальный aspect
    m_aspectRatio = m_window->GetAspectRatio();

    // 2. OpenGL контекст
    m_glContext = std::make_unique<OpenGLContext>(m_window->GetDrawingDC());
    if (!m_glContext->Initialize(4, 6, true))
    {
        std::cerr << "Не удалось инициализировать OpenGL" << std::endl;
        return false;
    }

    // 3. Рендерер, сцена, камера
    m_renderer = std::make_unique<Renderer>();
    m_camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 5.0f));
    m_scene = std::make_unique<Scene>();

    // Создаем тестовую сцену
    auto cube1Mesh = std::make_unique<Mesh>();
    auto cube1 = std::make_unique<Entity>("Cube1");
    cube1->AttachMesh(std::move(cube1Mesh));
    cube1->SetLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    m_scene->GetRoot()->AddChild(std::move(cube1));

    auto cube2Mesh = std::make_unique<Mesh>();
    auto cube2 = std::make_unique<Entity>("Cube2");
    cube2->AttachMesh(std::move(cube2Mesh));
    cube2->SetLocalPosition(glm::vec3(2.0f, 0.0f, 0.0f));
    cube2->SetLocalScale(glm::vec3(0.5f));
    m_scene->GetRoot()->GetChildren()[0]->AddChild(std::move(cube2));

    // 4. Настраиваем систему ввода
    SetupInputBindings();

    // Таймер для deltaTime
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_lastTime);

    m_running = true;
    return true;
}

void Engine::SetupInputBindings()
{
    auto& inputSystem = ogle::input::InputSystem::GetInstance();
    
    // Создаем контекст геймплея
    m_gameplayContext = inputSystem.CreateGameplayContext();
    if (!m_gameplayContext) 
    {
        std::cerr << "Failed to create gameplay context!" << std::endl;
        return;
    }
    
    // Убедимся, что оси созданы
    if (!m_gameplayContext->GetAxis("LookHorizontal") ||
        !m_gameplayContext->GetAxis("LookVertical") ||
        !m_gameplayContext->GetAxis("MoveHorizontal") ||
        !m_gameplayContext->GetAxis("MoveVertical"))
    {
        std::cerr << "Some input axes are missing!" << std::endl;
    }
    
    // Настраиваем оси для камеры - УВЕЛИЧИМ ЧУВСТВИТЕЛЬНОСТЬ
    auto* lookHorizontal = m_gameplayContext->GetAxis("LookHorizontal");
    if (lookHorizontal)
    {
        std::cout << "Setting up LookHorizontal axis" << std::endl;
        lookHorizontal->SetCallback([this](const glm::vec2& value, float deltaTime) {
            // Увеличим множитель для лучшей чувствительности
            m_camera->Rotate(value.x * 200.0f * deltaTime, 0.0f);
        });
    }
    
    auto* lookVertical = m_gameplayContext->GetAxis("LookVertical");
    if (lookVertical)
    {
        std::cout << "Setting up LookVertical axis" << std::endl;
        lookVertical->SetCallback([this](const glm::vec2& value, float deltaTime) {
            m_camera->Rotate(0.0f, value.y * 200.0f * deltaTime);
        });
    }
    
    // Оси движения камеры
    auto* moveHorizontal = m_gameplayContext->GetAxis("MoveHorizontal");
    if (moveHorizontal)
    {
        std::cout << "Setting up MoveHorizontal axis" << std::endl;
        moveHorizontal->SetCallback([this](const glm::vec2& value, float deltaTime) {
            float velocity = m_camera->GetMovementSpeed() * deltaTime;
            m_camera->MoveRight(value.x * velocity);
        });
    }
    
    auto* moveVertical = m_gameplayContext->GetAxis("MoveVertical");
    if (moveVertical)
    {
        std::cout << "Setting up MoveVertical axis" << std::endl;
        moveVertical->SetCallback([this](const glm::vec2& value, float deltaTime) {
            float velocity = m_camera->GetMovementSpeed() * deltaTime;
            m_camera->MoveForward(value.y * velocity);
        });
    }
    
    // Действия для камеры - ИЗМЕНИМ НА Pressed вместо Held
    auto* moveUp = m_gameplayContext->CreateAction("MoveUp");
    if (moveUp)
    {
        moveUp->AddKeyBinding('Q', ogle::input::ActionTrigger::Held);
        moveUp->AddKeyBinding(VK_SPACE, ogle::input::ActionTrigger::Held);
        moveUp->SetCallback([this](const ogle::input::InputEvent& event, float value) {
            // Для Held событий value будет 1.0
            float velocity = m_camera->GetMovementSpeed() * m_lastDeltaTime;
            m_camera->MoveUp(velocity);
        });
    }
    
    auto* moveDown = m_gameplayContext->CreateAction("MoveDown");
    if (moveDown)
    {
        moveDown->AddKeyBinding('E', ogle::input::ActionTrigger::Held);
        moveDown->AddKeyBinding(VK_CONTROL, ogle::input::ActionTrigger::Held);
        moveDown->SetCallback([this](const ogle::input::InputEvent& event, float value) {
            float velocity = m_camera->GetMovementSpeed() * m_lastDeltaTime;
            m_camera->MoveUp(-velocity);
        });
    }
    
    // Действие для ускорения
    auto* sprint = m_gameplayContext->CreateAction("Sprint");
    if (sprint)
    {
        sprint->AddKeyBinding(VK_SHIFT, ogle::input::ActionTrigger::Pressed);
        sprint->SetCallback([this](const ogle::input::InputEvent& event, float value) {
            if (event.type == ogle::input::EventType::KeyPressed)
            {
                m_camera->SetMovementSpeed(15.0f);
            }
            else if (event.type == ogle::input::EventType::KeyReleased)
            {
                m_camera->SetMovementSpeed(5.0f);
            }
        });
    }
    
    // Действие для захвата мыши - УПРОСТИМ
    auto* toggleMouseCapture = m_gameplayContext->CreateAction("ToggleMouseCapture");
    if (toggleMouseCapture)
    {
        toggleMouseCapture->AddMouseBinding(1, ogle::input::ActionTrigger::Pressed);
        toggleMouseCapture->AddKeyBinding(VK_ESCAPE, ogle::input::ActionTrigger::Pressed);
        
        toggleMouseCapture->SetCallback([this](const ogle::input::InputEvent& event, float value) {
            auto& inputSystem = ogle::input::InputSystem::GetInstance();
            if (event.type == ogle::input::EventType::MouseButtonPressed)
            {
                std::cout << "Mouse capture ON" << std::endl;
                inputSystem.SetMouseCapture(true);
            }
            else if (event.type == ogle::input::EventType::KeyPressed)
            {
                std::cout << "Mouse capture OFF" << std::endl;
                inputSystem.SetMouseCapture(false);
            }
        });
    }
    
    std::cout << "Input bindings configured successfully" << std::endl;
}
// УДАЛИТЕ ВСЁ ОТСЮДА ДО КОНЦА ФУНКЦИИ SetupInputBindings() 
// (строки 169-267 из вашего файла)

void Engine::HandleWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Передаем сообщения в новую систему ввода
    auto& inputSystem = ogle::input::InputSystem::GetInstance();
    inputSystem.ProcessWindowMessage(msg, wParam, lParam);
}

int Engine::Run()
{
    m_window->Show(SW_SHOW);

    MSG msg{};
    while (m_running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                m_running = false;
                break;
            }
        }

        if (!m_running) break;

        // DeltaTime
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double deltaTime = static_cast<double>(now.QuadPart - m_lastTime.QuadPart) / m_frequency.QuadPart;
        m_lastTime = now;
        
        m_lastDeltaTime = static_cast<float>(deltaTime);

        Update(deltaTime);
        Render();
    }

    return static_cast<int>(msg.wParam);
}

void Engine::Shutdown()
{
    // Освобождаем ресурсы системы ввода
    auto& inputSystem = ogle::input::InputSystem::GetInstance();
    inputSystem.Shutdown();
    
    m_glContext.reset();
    m_window.reset();
}

void Engine::Update(double deltaTime)
{
    // Обновляем систему ввода
    auto& inputSystem = ogle::input::InputSystem::GetInstance();
    inputSystem.Update(static_cast<float>(deltaTime));
    
    // Обновляем сцену
    m_scene->Update(deltaTime);
    
    // Обновляем камеру (если нужно)
    m_camera->Update();
}

void Engine::Render()
{
    m_glContext->MakeCurrent();

    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Используем правильное соотношение сторон
    // TODO: Передать m_aspectRatio в рендерер
    m_renderer->Render(*m_scene, *m_camera);

    m_glContext->SwapBuffers();
}