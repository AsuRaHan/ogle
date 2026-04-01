#include "config/ConfigManager.h"

#include "Logger.h"
#include "core/FileSystem.h"

#include <nlohmann/json.hpp>
#include <windows.h>

namespace
{
    std::string Narrow(const std::wstring& value)
    {
        if (value.empty()) {
            return {};
        }

        const int sizeNeeded = WideCharToMultiByte(
            CP_UTF8,
            0,
            value.c_str(),
            static_cast<int>(value.size()),
            nullptr,
            0,
            nullptr,
            nullptr);

        if (sizeNeeded <= 0) {
            return {};
        }

        std::string result(static_cast<std::size_t>(sizeNeeded), '\0');
        WideCharToMultiByte(
            CP_UTF8,
            0,
            value.c_str(),
            static_cast<int>(value.size()),
            result.data(),
            sizeNeeded,
            nullptr,
            nullptr);
        return result;
    }

    std::wstring Widen(const std::string& value)
    {
        if (value.empty()) {
            return {};
        }

        const int sizeNeeded = MultiByteToWideChar(
            CP_UTF8,
            0,
            value.c_str(),
            static_cast<int>(value.size()),
            nullptr,
            0);

        if (sizeNeeded <= 0) {
            return {};
        }

        std::wstring result(static_cast<std::size_t>(sizeNeeded), L'\0');
        MultiByteToWideChar(
            CP_UTF8,
            0,
            value.c_str(),
            static_cast<int>(value.size()),
            result.data(),
            sizeNeeded);
        return result;
    }
}

ConfigManager::ConfigManager(std::filesystem::path configPath)
    : m_configPath(std::move(configPath))
{
}

bool ConfigManager::Load()
{
    const std::filesystem::path resolvedPath = FileSystem::ResolvePath(m_configPath);
    std::string content;
    if (!FileSystem::ReadTextFile(resolvedPath, content)) {
        return false;
    }

    nlohmann::json json = nlohmann::json::parse(content, nullptr, false);
    if (json.is_discarded()) {
        LOG_WARN("Config file is invalid JSON: " + resolvedPath.string());
        return false;
    }

    AppConfig loadedConfig = CreateDefaultConfig();

    if (json.contains("window")) {
        const auto& window = json["window"];
        loadedConfig.window.title = Widen(window.value("title", Narrow(loadedConfig.window.title)));
        loadedConfig.window.width = window.value("width", loadedConfig.window.width);
        loadedConfig.window.height = window.value("height", loadedConfig.window.height);
    }

    if (json.contains("editor")) {
        const auto& editor = json["editor"];
        loadedConfig.editor.enabled = editor.value("enabled", loadedConfig.editor.enabled);
    }

    if (json.contains("world")) {
        const auto& world = json["world"];
        loadedConfig.world.path = world.value("path", loadedConfig.world.path);
        loadedConfig.world.loadOnStartup = world.value("loadOnStartup", loadedConfig.world.loadOnStartup);
        loadedConfig.world.saveDefaultWorldIfMissing = world.value("saveDefaultWorldIfMissing", loadedConfig.world.saveDefaultWorldIfMissing);
    }

    if (json.contains("assets")) {
        const auto& assets = json["assets"];
        loadedConfig.assets.path = assets.value("path", loadedConfig.assets.path);
    }

    if (json.contains("scripts")) {
        const auto& scripts = json["scripts"];
        loadedConfig.scripts.runStartupScript = scripts.value("runStartupScript", loadedConfig.scripts.runStartupScript);
        loadedConfig.scripts.startupScriptPath = scripts.value("startupScriptPath", loadedConfig.scripts.startupScriptPath);
    }

    m_config = loadedConfig;
    m_configPath = resolvedPath;
    LOG_INFO("Config loaded: " + resolvedPath.string());
    return true;
}

bool ConfigManager::Save() const
{
    nlohmann::json json;
    json["window"] = {
        { "title", Narrow(m_config.window.title) },
        { "width", m_config.window.width },
        { "height", m_config.window.height }
    };
    json["editor"] = {
        { "enabled", m_config.editor.enabled }
    };
    json["world"] = {
        { "path", m_config.world.path },
        { "loadOnStartup", m_config.world.loadOnStartup },
        { "saveDefaultWorldIfMissing", m_config.world.saveDefaultWorldIfMissing }
    };
    json["assets"] = {
        { "path", m_config.assets.path }
    };
    json["scripts"] = {
        { "runStartupScript", m_config.scripts.runStartupScript },
        { "startupScriptPath", m_config.scripts.startupScriptPath }
    };

    const std::filesystem::path resolvedPath = FileSystem::ResolvePath(m_configPath);
    const bool saved = FileSystem::WriteTextFile(resolvedPath, json.dump(4));
    if (saved) {
        LOG_INFO("Config saved: " + resolvedPath.string());
    } else {
        LOG_ERROR("Failed to save config: " + resolvedPath.string());
    }

    return saved;
}

bool ConfigManager::LoadOrCreateDefault()
{
    if (Load()) {
        return true;
    }

    m_config = CreateDefaultConfig();
    return Save();
}

const AppConfig& ConfigManager::GetConfig() const
{
    return m_config;
}

AppConfig& ConfigManager::GetConfig()
{
    return m_config;
}

const std::filesystem::path& ConfigManager::GetConfigPath() const
{
    return m_configPath;
}

AppConfig ConfigManager::CreateDefaultConfig()
{
    return AppConfig{};
}
