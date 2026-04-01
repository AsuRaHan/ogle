#pragma once

#include <string>

struct AppConfig
{
    struct WindowSettings {
        std::wstring title = L"Main window";
        int width = 900;
        int height = 600;
    } window;

    struct EditorSettings {
        bool enabled = true;
    } editor;

    struct WorldSettings {
        std::string path = "data/worlds/default_world.json";
        bool loadOnStartup = true;
        bool saveDefaultWorldIfMissing = true;
    } world;

    struct ScriptSettings {
        bool runStartupScript = false;
        std::string startupScriptPath = "scripts/test_world.js";
    } scripts;
};
