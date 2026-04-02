#pragma once

#include <filesystem>

struct EditorState;
class ConfigManager;

class EditorContentBrowserPanel
{
public:
    void Draw(EditorState& state, ConfigManager& configManager);

private:
    void DrawDirectory(
        EditorState& state,
        const std::filesystem::path& directoryPath,
        const std::filesystem::path& rootPath);
    void HandleFileSelected(
        EditorState& state,
        const std::filesystem::path& filePath,
        const std::filesystem::path& rootPath);
};
