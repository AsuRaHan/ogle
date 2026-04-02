#include "editor/EditorAssetHelpers.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace
{
    constexpr const char* kContentBrowserAssetPayload = "OGLE_CONTENT_BROWSER_ASSET";
}

const char* GetContentBrowserAssetPayload()
{
    return kContentBrowserAssetPayload;
}

bool IsEditorModelAssetPath(const std::string& path)
{
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    return extension == ".obj" ||
        extension == ".fbx" ||
        extension == ".glb" ||
        extension == ".gltf" ||
        extension == ".stl";
}

bool IsEditorTextureAssetPath(const std::string& path)
{
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    return extension == ".png" ||
        extension == ".jpg" ||
        extension == ".jpeg" ||
        extension == ".bmp" ||
        extension == ".tga";
}

std::string BuildEditorEntityNameFromAssetPath(const std::string& path)
{
    const std::filesystem::path assetPath(path);
    return assetPath.stem().string().empty() ? "Model" : assetPath.stem().string();
}
