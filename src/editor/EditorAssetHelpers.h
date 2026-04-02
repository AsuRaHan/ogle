#pragma once

#include <string>

const char* GetContentBrowserAssetPayload();
bool IsEditorModelAssetPath(const std::string& path);
bool IsEditorTextureAssetPath(const std::string& path);
std::string BuildEditorEntityNameFromAssetPath(const std::string& path);
