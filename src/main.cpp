#include "App.h"
#include "config/ConfigManager.h"
#include "ui/Win32Window.h"
#include "Logger.h"
#include <memory>
#include <windows.h>

class MainAppWindow : public Win32Window
{
protected:
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            default:
                return Win32Window::HandleMessage(msg, wParam, lParam);
        }
    }
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    HMODULE user32Module = GetModuleHandleW(L"user32.dll");
    if (user32Module != nullptr)
    {
        using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(HANDLE);
        auto setProcessDpiAwarenessContext =
            reinterpret_cast<SetProcessDpiAwarenessContextFn>(
                GetProcAddress(user32Module, "SetProcessDpiAwarenessContext"));

        if (setProcessDpiAwarenessContext != nullptr)
        {
            setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
        else
        {
            SetProcessDPIAware();
        }
    }

    if (!Logger::Instance().Init(L"log.txt"))
    {
        return -1;
    }
    Logger::Instance().SetLevel(Logger::Level::Debug);

    LOG_INFO("Application start");

    ConfigManager configManager;
    if (!configManager.LoadOrCreateDefault())
    {
        LOG_WARN("Failed to load config, using defaults");
    }

    const AppConfig& config = configManager.GetConfig();

    const std::string stateFile = "window_state.txt";
    auto mainWindow = std::make_unique<MainAppWindow>();
    mainWindow->SetTitle(config.window.title);
    mainWindow->SetSize(config.window.width, config.window.height);
    mainWindow->SetStyle(WS_OVERLAPPEDWINDOW);
    mainWindow->SetExStyle(0);

    // Save window state before moving the window into App
    mainWindow->SaveWindowState(stateFile);
    App app(std::move(mainWindow), std::move(configManager));
    int appResult = app.Run(hInstance, nCmdShow);

    LOG_INFO("Application exit: " + std::to_string(appResult));
    // Window state already saved before run; no need to save again

    Logger::Instance().Shutdown();
    return appResult;
}

