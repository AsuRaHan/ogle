#pragma once

#include "config/AppConfig.h"

#include <filesystem>

class ConfigManager
{
public:
    ConfigManager() = default;
    explicit ConfigManager(std::filesystem::path configPath);

    bool Load();
    bool Save() const;
    bool LoadOrCreateDefault();

    const AppConfig& GetConfig() const;
    AppConfig& GetConfig();
    const std::filesystem::path& GetConfigPath() const;

private:
    static AppConfig CreateDefaultConfig();

    std::filesystem::path m_configPath = "config/app_config.json";
    AppConfig m_config = CreateDefaultConfig();
};
