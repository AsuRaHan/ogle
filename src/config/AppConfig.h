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
        std::string path = "assets/worlds/default_world.json";
        bool loadOnStartup = true;
        bool saveDefaultWorldIfMissing = true;
    } world;

    struct AssetsSettings {
        std::string path = "assets";
    } assets;

    struct ScriptSettings {
        bool runStartupScript = false;
        std::string startupScriptPath = "assets/scripts/test_world.js";
    } scripts;
};
