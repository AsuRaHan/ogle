#include "editor/EditorContentBrowserPanel.h"

#include "editor/EditorAssetHelpers.h"
#include "editor/EditorState.h"
#include "config/ConfigManager.h"

#include <imgui.h>
#include <algorithm>
#include <cstring>
#include <vector>

void EditorContentBrowserPanel::Draw(EditorState& state, ConfigManager& configManager)
{
    ImGui::InputText("Assets Root", state.assetsPathBuffer.data(), state.assetsPathBuffer.size());

    if (ImGui::Button("Save Assets Root")) {
        configManager.GetConfig().assets.path = state.assetsPathBuffer.data();
        configManager.Save();
    }

    const std::filesystem::path rootPath = std::filesystem::path(state.assetsPathBuffer.data());

    if (state.contentSelectionBuffer[0] != '\0') {
        ImGui::TextWrapped("Selected asset: %s", state.contentSelectionBuffer.data());
    }

    if (!std::filesystem::exists(rootPath)) {
        ImGui::TextDisabled("Folder not found: %s", rootPath.string().c_str());
        return;
    }

    ImGui::BeginChild("ContentBrowserTree", ImVec2(0.0f, 220.0f), true);
    DrawDirectory(state, rootPath, rootPath);
    ImGui::EndChild();
}

void EditorContentBrowserPanel::DrawDirectory(
    EditorState& state,
    const std::filesystem::path& directoryPath,
    const std::filesystem::path& rootPath)
{
    std::vector<std::filesystem::directory_entry> entries;
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        entries.push_back(entry);
    }

    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.is_directory() != b.is_directory()) {
            return a.is_directory() > b.is_directory();
        }
        return a.path().filename().string() < b.path().filename().string();
    });

    for (const auto& entry : entries) {
        const std::filesystem::path entryPath = entry.path();
        const std::string label = entryPath.filename().string();

        if (entry.is_directory()) {
            const bool opened = ImGui::TreeNode(label.c_str());
            if (opened) {
                DrawDirectory(state, entryPath, rootPath);
                ImGui::TreePop();
            }
        } else {
            const std::string relativePath = std::filesystem::relative(entryPath, rootPath).generic_string();
            const bool selected = std::strcmp(state.contentSelectionBuffer.data(), relativePath.c_str()) == 0;
            if (ImGui::Selectable((label + "##" + relativePath).c_str(), selected)) {
                HandleFileSelected(state, entryPath, rootPath);
            }

            const std::string payloadPath = entryPath.generic_string();
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload(
                    GetContentBrowserAssetPayload(),
                    payloadPath.c_str(),
                    payloadPath.size() + 1);
                ImGui::TextUnformatted(label.c_str());
                ImGui::TextDisabled("%s", payloadPath.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
}

void EditorContentBrowserPanel::HandleFileSelected(
    EditorState& state,
    const std::filesystem::path& filePath,
    const std::filesystem::path& rootPath)
{
    std::filesystem::path pathForUse = filePath;
    std::error_code errorCode;
    const std::filesystem::path relativeToCwd = std::filesystem::relative(filePath, std::filesystem::current_path(), errorCode);
    const std::string relativeToCwdString = relativeToCwd.generic_string();
    if (!errorCode && !relativeToCwd.empty() && relativeToCwdString.rfind("../", 0) != 0 && relativeToCwdString != "..") {
        pathForUse = relativeToCwd;
    }

    const std::string usePathString = pathForUse.generic_string();

    state.contentSelectionBuffer.fill('\0');
    std::strncpy(state.contentSelectionBuffer.data(), usePathString.c_str(), state.contentSelectionBuffer.size() - 1);

    if (IsEditorTextureAssetPath(filePath.generic_string())) {
        state.createTexturePathBuffer.fill('\0');
        state.texturePathBuffer.fill('\0');
        std::strncpy(state.createTexturePathBuffer.data(), usePathString.c_str(), state.createTexturePathBuffer.size() - 1);
        std::strncpy(state.texturePathBuffer.data(), usePathString.c_str(), state.texturePathBuffer.size() - 1);
    }

    if (IsEditorModelAssetPath(filePath.generic_string())) {
        state.createModelPathBuffer.fill('\0');
        std::strncpy(state.createModelPathBuffer.data(), usePathString.c_str(), state.createModelPathBuffer.size() - 1);
    }
}
