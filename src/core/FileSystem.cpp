#include "core/FileSystem.h"

#include <fstream>
#include <sstream>
#include <windows.h>

bool FileSystem::Exists(const std::filesystem::path& path)
{
    std::error_code errorCode;
    return std::filesystem::exists(path, errorCode);
}

bool FileSystem::EnsureDirectory(const std::filesystem::path& path)
{
    if (path.empty()) {
        return false;
    }

    std::error_code errorCode;
    if (std::filesystem::exists(path, errorCode)) {
        return std::filesystem::is_directory(path, errorCode);
    }

    return std::filesystem::create_directories(path, errorCode);
}

bool FileSystem::EnsureParentDirectory(const std::filesystem::path& filePath)
{
    const std::filesystem::path parentPath = filePath.parent_path();
    if (parentPath.empty()) {
        return true;
    }

    return EnsureDirectory(parentPath);
}

bool FileSystem::ReadTextFile(const std::filesystem::path& path, std::string& content)
{
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    content = buffer.str();
    return true;
}

bool FileSystem::WriteTextFile(const std::filesystem::path& path, const std::string& content)
{
    if (!EnsureParentDirectory(path)) {
        return false;
    }

    std::ofstream output(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }

    output << content;
    return output.good();
}

std::filesystem::path FileSystem::GetWorkingDirectory()
{
    std::error_code errorCode;
    return std::filesystem::current_path(errorCode);
}

std::filesystem::path FileSystem::GetExecutableDirectory()
{
    char moduleFileName[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, moduleFileName, MAX_PATH) == 0) {
        return {};
    }

    return std::filesystem::path(moduleFileName).parent_path();
}

std::filesystem::path FileSystem::ResolvePath(const std::filesystem::path& path)
{
    if (path.is_absolute()) {
        return path;
    }

    const std::filesystem::path workingCandidate = GetWorkingDirectory() / path;
    if (Exists(workingCandidate)) {
        return workingCandidate;
    }

    const std::filesystem::path executableCandidate = GetExecutableDirectory() / path;
    if (Exists(executableCandidate)) {
        return executableCandidate;
    }

    return GetWorkingDirectory() / path;
}
