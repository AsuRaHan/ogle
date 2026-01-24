// src/core/Engine.cpp
#include "Engine.h"

namespace ogle {

Engine::Engine(HINSTANCE hInstance) : m_hInstance(hInstance) {}

Engine::~Engine() {
    Shutdown();
}

// src/core/Engine.cpp — метод Initialize

bool Engine::Initialize() {
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_lastTime);

    // 1. Окно
    auto* window = RegisterSystem<WindowSystem>(m_hInstance, L"OGLE 3D Engine", 1280, 720);
    if (!window || !window->InitWindow()) {
        Logger::Error("WindowSystem init failed");
        return false;
    }

    // 2. Рендер — HDC передаётся в конструктор
    auto* render = RegisterSystem<RenderSystem>(window->GetHDC());
    if (!render) {
        Logger::Error("RenderSystem registration failed");
        return false;
    }

    // Подписываемся на ресайз окна (lambda, без зависимостей)
    window->AddResizeListener([render](int w, int h) {
        render->OnResize(w, h);
    });

    // 3. Инициализация всех систем (RenderSystem уже создан, но Initialize ещё нет)
    for (auto& sys : m_systems) {
        if (!sys->Initialize()) {
            Logger::Error("Initialize failed for " + sys->GetName());
            return false;
        }
    }

    m_running = true;
    Logger::Info("Engine initialized successfully");
    return true;
}



int Engine::Run() {
    auto* window = GetSystem<WindowSystem>();
    if (window) {
        window->Show(SW_SHOW);
    } else {
        Logger::Error("No WindowSystem found");
        return 1;
    }

    MSG msg{};
    while (m_running) {
        ProcessMessages();

        if (!m_running) break;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        m_deltaTime = static_cast<float>(
            static_cast<double>(now.QuadPart - m_lastTime.QuadPart) / m_frequency.QuadPart);
        m_lastTime = now;

        UpdateSystems();
        RenderSystems();
    }

    return 0;
}

void Engine::ProcessMessages() {
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // Рассылаем сообщение всем системам
        for (auto& sys : m_systems) {
            sys->OnWindowMessage(msg.message, msg.wParam, msg.lParam);
        }

        // Engine сам реагирует на WM_QUIT
        if (msg.message == WM_QUIT) {
            m_running = false;
        }
    }
}

void Engine::UpdateSystems() {
    for (auto& sys : m_systems) {
        sys->Update(m_deltaTime);
    }
}

void Engine::RenderSystems() {
    for (auto& sys : m_systems) {
        sys->Render();
    }
}

void Engine::Shutdown() {
    // Shutdown в обратном порядке
    for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
        (*it)->Shutdown();
    }
    m_systems.clear();
    Logger::Info("Engine shutdown");
}

} // namespace ogle