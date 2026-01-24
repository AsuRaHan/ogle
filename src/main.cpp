#include <iostream>
#include "log/Logger.h"
#include "core/Engine.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    // #ifdef _DEBUG
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    ogle::Logger::Info("Debug console activate");
    // #endif

    Engine engine(hInstance);

    if (!engine.Initialize())
    {
        return 1;
    }
    FreeConsole();
    return engine.Run();
}