#pragma once

#include <filesystem>
#include <string>

class FileSystem
{
public:
    static bool Exists(const std::filesystem::path& path);
    static bool EnsureDirectory(const std::filesystem::path& path);
    static bool EnsureParentDirectory(const std::filesystem::path& filePath);
    static bool ReadTextFile(const std::filesystem::path& path, std::string& content);
    static bool WriteTextFile(const std::filesystem::path& path, const std::string& content);
    static std::filesystem::path GetWorkingDirectory();
    static std::filesystem::path GetExecutableDirectory();
    static std::filesystem::path ResolvePath(const std::filesystem::path& path);
};
