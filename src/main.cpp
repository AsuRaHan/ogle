#include "engine/Engine.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    #ifdef _DEBUG
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    //std::cout << "Debug консоль включена" << std::endl;
    #endif

    Engine engine(hInstance);

    if (!engine.Initialize())
    {
        return 1;
    }

    return engine.Run();
}