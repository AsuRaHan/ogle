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
    // m_scene = std::make_unique<Scene>();
    m_camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 5.0f));  // дальше от куба

    m_scene = std::make_unique<Scene>();

    // Первый кубик — прямой ребёнок root
    auto cube1Mesh = std::make_unique<Mesh>();
    auto cube1 = std::make_unique<Entity>("Cube1");
    cube1->AttachMesh(std::move(cube1Mesh));
    cube1->SetLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    m_scene->GetRoot()->AddChild(std::move(cube1));

    // Второй кубик — ребёнок первого кубика (иерархия!)
    auto cube2Mesh = std::make_unique<Mesh>();
    auto cube2 = std::make_unique<Entity>("Cube2");
    cube2->AttachMesh(std::move(cube2Mesh));
    cube2->SetLocalPosition(glm::vec3(2.0f, 0.0f, 0.0f));  // относительно родителя
    cube2->SetLocalScale(glm::vec3(0.5f));
    m_scene->GetRoot()->GetChildren()[0]->AddChild(std::move(cube2));  // или сохрани указатель



    // Таймер для deltaTime
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_lastTime);

    m_running = true;
    return true;
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

        ProcessInput();
        Update(deltaTime);
        Render();
    }

    return static_cast<int>(msg.wParam);
}

void Engine::Shutdown()
{
    m_glContext.reset();
    m_window.reset();
    // остальные reset, если нужно
}

void Engine::ProcessInput()
{
    // Здесь обработка клавиатуры/мыши (позже отдельный класс Input)
    // Пример: if (GetAsyncKeyState(VK_UP) & 0x8000) m_camera->MoveForward();
}

void Engine::Update(double deltaTime)
{
    m_scene->Update(deltaTime);
    m_camera->Update();
}

void Engine::Render()
{
    m_glContext->MakeCurrent();

    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_renderer->Render(*m_scene, *m_camera);

    m_glContext->SwapBuffers();
}