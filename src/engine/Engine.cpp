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
    
    // Инициализируем систему ввода
    auto& inputSystem = ogle::input::InputSystem::GetInstance();
    inputSystem.Initialize(m_window->GetHWND());
    
    // Настраиваем callback'ы окна: все сообщения идут в HandleWindowMessage
    m_window->SetMessageCallback([this](UINT msg, WPARAM wParam, LPARAM lParam) {
        this->HandleWindowMessage(msg, wParam, lParam);
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

    // 4. Настраиваем систему ввода через конфигуратор
    m_inputConfigurator = std::make_unique<ogle::InputConfigurator>();
    m_inputConfigurator->ConfigureCameraControls(
        inputSystem, 
        *m_camera, 
        m_lastDeltaTime
    );

    // Таймер для deltaTime
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_lastTime);

    m_running = true;
    std::cout << "Engine initialized successfully" << std::endl;
    return true;
}

void Engine::HandleWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Передаем сообщения в систему ввода
    auto& inputSystem = ogle::input::InputSystem::GetInstance();
    inputSystem.ProcessWindowMessage(msg, wParam, lParam);
    
    // Обработка изменения размера: обновляем aspectRatio здесь вместо отдельного колбека
    if (msg == WM_SIZE)
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if (height != 0)
        {
            m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
            std::cout << "Окно ресайзнуто: новый aspect = " << m_aspectRatio << std::endl;
        }
    }
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
    m_inputConfigurator.reset();
    
    std::cout << "Engine shutdown" << std::endl;
}

void Engine::Update(double deltaTime)
{
    // Обновляем систему ввода
    auto& inputSystem = ogle::input::InputSystem::GetInstance();
    inputSystem.Update(static_cast<float>(deltaTime));
    
    // Обновляем конфигуратор ввода (если нужно)
    if (m_inputConfigurator)
    {
        m_inputConfigurator->Update(static_cast<float>(deltaTime));
    }
    
    // Обновляем сцену
    m_scene->Update(deltaTime);
    
    // Обновляем камеру
    m_camera->Update();
}

void Engine::Render()
{
    m_glContext->MakeCurrent();

    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Рендерим сцену
    m_renderer->Render(*m_scene, *m_camera);

    m_glContext->SwapBuffers();
}