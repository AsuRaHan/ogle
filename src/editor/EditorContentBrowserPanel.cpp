#include "editor/EditorContentBrowserPanel.h"

#include "editor/EditorAssetHelpers.h"
#include "editor/EditorState.h"
#include "config/ConfigManager.h"
#include "managers/WorldManager.h"
#include "world/WorldComponents.h"
#include "render/AnimationLibrary.h"
#include "core/FileSystem.h"
#include "../Logger.h"

#include <imgui.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <vector>

static std::string NormalizePath(const std::string& path)
{
    std::filesystem::path p(path);
    if (!p.is_absolute()) {
        p = std::filesystem::current_path() / p;
    }
    return std::filesystem::weakly_canonical(p).generic_string();
}

static bool QueryModelMetadata(
    const std::string& path,
    int& meshCount,
    int& materialCount,
    int& boneCount,
    int& animationCount,
    std::vector<OGLE::AnimationClip>& animationClips)
{
    meshCount = materialCount = boneCount = animationCount = 0;
    animationClips.clear();

    std::filesystem::path filePath(path);
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (ext == ".omdl") {
        std::string content;
        if (!FileSystem::ReadTextFile(filePath, content)) {
            return false;
        }

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(content);
        } catch (...) {
            return false;
        }

        if (!j.contains("mesh")) {
            return false;
        }

        meshCount = 1;
        materialCount = 0;

        if (j.contains("skeleton") && j["skeleton"].contains("boneCount")) {
            boneCount = j["skeleton"]["boneCount"].get<int>();
        }

        if (j.contains("animationClips") && j["animationClips"].is_array()) {
            animationCount = static_cast<int>(j["animationClips"].size());
            for (const auto& clipJson : j["animationClips"]) {
                OGLE::AnimationClip clip;
                clip.name = clipJson.value("name", std::string(""));
                clip.duration = clipJson.value("duration", 1.0f);
                clip.ticksPerSecond = clipJson.value("ticksPerSecond", 24.0f);

                if (clipJson.contains("tracks") && clipJson["tracks"].is_array()) {
                    for (const auto& trackJson : clipJson["tracks"]) {
                        OGLE::AnimationTrack track;
                        track.nodeName = trackJson.value("nodeName", std::string(""));
                        if (trackJson.contains("keyframes") && trackJson["keyframes"].is_array()) {
                            for (const auto& keyJson : trackJson["keyframes"]) {
                                OGLE::AnimationKeyframe key;
                                key.time = keyJson.value("time", 0.0f);
                                if (keyJson.contains("translation") && keyJson["translation"].is_array() && keyJson["translation"].size() == 3) {
                                    key.translation = glm::vec3(
                                        keyJson["translation"][0].get<float>(),
                                        keyJson["translation"][1].get<float>(),
                                        keyJson["translation"][2].get<float>());
                                }
                                if (keyJson.contains("rotation") && keyJson["rotation"].is_array() && keyJson["rotation"].size() == 4) {
                                    key.rotation = glm::quat(
                                        keyJson["rotation"][0].get<float>(),
                                        keyJson["rotation"][1].get<float>(),
                                        keyJson["rotation"][2].get<float>(),
                                        keyJson["rotation"][3].get<float>());
                                }
                                if (keyJson.contains("scale") && keyJson["scale"].is_array() && keyJson["scale"].size() == 3) {
                                    key.scale = glm::vec3(
                                        keyJson["scale"][0].get<float>(),
                                        keyJson["scale"][1].get<float>(),
                                        keyJson["scale"][2].get<float>());
                                }
                                track.keyframes.push_back(std::move(key));
                            }
                        }
                        clip.tracks.push_back(std::move(track));
                    }
                }

                animationClips.push_back(std::move(clip));
            }
        }

        return true;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_FlipUVs);

    if (!scene) {
        return false;
    }

    meshCount = scene->mNumMeshes;
    materialCount = scene->mNumMaterials;

    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        if (!mesh) continue;
        boneCount += mesh->mNumBones;
    }

    animationCount = scene->mNumAnimations;
    for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex) {
        const aiAnimation* aiAnim = scene->mAnimations[animIndex];
        if (!aiAnim) continue;

        OGLE::AnimationClip clip;
        clip.name = aiAnim->mName.C_Str();
        if (clip.name.empty()) {
            clip.name = "clip_" + std::to_string(animIndex);
        }
        clip.ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond != 0.0 ? aiAnim->mTicksPerSecond : 24.0);
        clip.duration = static_cast<float>(aiAnim->mDuration / clip.ticksPerSecond);
        if (clip.duration <= 0.0f) {
            clip.duration = 1.0f;
        }

        for (unsigned int channelIndex = 0; channelIndex < aiAnim->mNumChannels; ++channelIndex) {
            const aiNodeAnim* nodeAnim = aiAnim->mChannels[channelIndex];
            if (!nodeAnim) continue;

            OGLE::AnimationTrack track;
            track.nodeName = nodeAnim->mNodeName.C_Str();

            // convert keyframes
            std::map<float, OGLE::AnimationKeyframe> keyframesByTime;
            for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
                float t = static_cast<float>(nodeAnim->mPositionKeys[i].mTime / clip.ticksPerSecond);
                auto& frame = keyframesByTime[t];
                frame.time = t;
                frame.translation = glm::vec3(nodeAnim->mPositionKeys[i].mValue.x, nodeAnim->mPositionKeys[i].mValue.y, nodeAnim->mPositionKeys[i].mValue.z);
            }
            for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys; ++i) {
                float t = static_cast<float>(nodeAnim->mRotationKeys[i].mTime / clip.ticksPerSecond);
                auto& frame = keyframesByTime[t];
                frame.time = t;
                frame.rotation = glm::quat(nodeAnim->mRotationKeys[i].mValue.w, nodeAnim->mRotationKeys[i].mValue.x, nodeAnim->mRotationKeys[i].mValue.y, nodeAnim->mRotationKeys[i].mValue.z);
            }
            for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys; ++i) {
                float t = static_cast<float>(nodeAnim->mScalingKeys[i].mTime / clip.ticksPerSecond);
                auto& frame = keyframesByTime[t];
                frame.time = t;
                frame.scale = glm::vec3(nodeAnim->mScalingKeys[i].mValue.x, nodeAnim->mScalingKeys[i].mValue.y, nodeAnim->mScalingKeys[i].mValue.z);
            }

            for (auto& kv : keyframesByTime) {
                track.keyframes.push_back(kv.second);
            }

            clip.tracks.push_back(std::move(track));
        }

        animationClips.push_back(std::move(clip));
    }

    return true;
}

void EditorContentBrowserPanel::Draw(EditorState& state, WorldManager& worldManager, ConfigManager& configManager)
{
    ImGui::InputText("Assets Root", state.assetsPathBuffer.data(), state.assetsPathBuffer.size());

    if (ImGui::Button("Save Assets Root")) {
        configManager.GetConfig().assets.path = state.assetsPathBuffer.data();
        configManager.Save();
    }

    const std::filesystem::path rootPath = std::filesystem::path(state.assetsPathBuffer.data());

    if (state.contentSelectionBuffer[0] != '\0') {
        ImGui::TextWrapped("Selected asset: %s", state.contentSelectionBuffer.data());

        std::string selectedFile = state.contentSelectionBuffer.data();
        std::filesystem::path selectedPath = NormalizePath(selectedFile);
        if (std::filesystem::exists(selectedPath)) {
            try {
                auto fileSize = std::filesystem::file_size(selectedPath);
                ImGui::Text("File size: %.2f KB", static_cast<double>(fileSize) / 1024.0);
            } catch (const std::exception&) {
                ImGui::Text("File size: (unknown)");
            }

            std::string extension = selectedPath.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (IsEditorModelAssetPath(selectedPath.generic_string())) {
                int meshCount = 0, materialCount = 0, boneCount = 0, animationCount = 0;
                std::vector<OGLE::AnimationClip> modelClips;
                if (QueryModelMetadata(selectedPath.generic_string(), meshCount, materialCount, boneCount, animationCount, modelClips)) {
                    ImGui::Text("Model details: Meshes=%d, Materials=%d, Bones=%d, Animations=%d", meshCount, materialCount, boneCount, animationCount);

                    if (ImGui::Button("Load model to scene")) {
                        std::string entityName = BuildEditorEntityNameFromAssetPath(selectedPath.generic_string());
                        worldManager.GetActiveWorld().CreateModelFromFile(selectedPath.generic_string(), OGLE::ModelType::DYNAMIC, entityName);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Export as .omdl")) {
                        auto tempModel = std::make_shared<OGLE::ModelEntity>(OGLE::ModelType::DYNAMIC, selectedPath.generic_string());
                        if (tempModel->LoadFromFile(selectedPath.generic_string())) {
                            auto outputPath = selectedPath.parent_path() / (selectedPath.stem().string() + ".omdl");
                            if (tempModel->SaveToCustomFile(outputPath.generic_string())) {
                                LOG_INFO("Exported model to "+ outputPath.generic_string());
                            } else {
                                LOG_ERROR("Failed to export model to "+ outputPath.generic_string());
                            }
                        } else {
                            LOG_ERROR("Failed to load model for export: "+ selectedPath.generic_string());
                        }
                    }
                    ImGui::SameLine();
                    if (animationCount > 0 && ImGui::Button("Import animations to library")) {
                        for (const auto& clip : modelClips) {
                            OGLE::AnimationComponent anim;
                            anim.enabled = true;
                            anim.playing = false;
                            anim.loop = true;
                            anim.currentTime = 0.0f;
                            anim.playbackSpeed = 1.0f;
                            anim.duration = clip.duration;
                            anim.currentClip = clip.name;
                            anim.clips.push_back(clip);
                            OGLE::AnimationLibrary::Instance().AddAnimation(clip.name, anim);
                        }
                    }

                    if (!modelClips.empty()) {
                        for (const auto& clip : modelClips) {
                            ImGui::Text("- %s (%.2fs, tracks %zu)", clip.name.c_str(), clip.duration, clip.tracks.size());
                        }
                    }
                } else {
                    ImGui::TextDisabled("Model metadata cannot be read.");
                }
            } else if (IsEditorTextureAssetPath(selectedPath.generic_string())) {
                ImGui::Text("Texture file: %s", selectedPath.filename().string().c_str());
                if (ImGui::Button("Convert texture (copy)")) {
                    auto dest = selectedPath.parent_path() / (selectedPath.stem().string() + "_converted" + selectedPath.extension().string());
                    std::error_code ec;
                    std::filesystem::copy_file(selectedPath, dest, std::filesystem::copy_options::overwrite_existing, ec);
                    if (ec.value() == 0) {
                        ImGui::OpenPopup("TextureConverted");
                    }
                }
                if (ImGui::BeginPopup("TextureConverted")) {
                    ImGui::Text("Texture converted and copied to [*]_converted");
                    if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
            } else {
                ImGui::TextDisabled("Selected asset is not a model/texture.");
            }
        }
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

            // Context menu for files
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Load to Scene")) {
                    HandleFileSelected(state, entryPath, rootPath);
                }
                if (ImGui::MenuItem("Copy Path")) {
                    ImGui::SetClipboardText(entryPath.generic_string().c_str());
                }
                if (ImGui::MenuItem("Delete File")) {
                    try {
                        std::filesystem::remove(entryPath);
                        LOG_INFO("File deleted: " + entryPath.generic_string());
                    } catch (const std::exception& ex) {
                        LOG_ERROR("Failed to delete file: " + std::string(ex.what()));
                    }
                }
                ImGui::EndPopup();
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
        std::strncpy(state.createTexturePathBuffer.data(), usePathString.c_str(), state.createTexturePathBuffer.size() - 1);
    }

    if (IsEditorModelAssetPath(filePath.generic_string())) {
        state.createModelPathBuffer.fill('\0');
        std::strncpy(state.createModelPathBuffer.data(), usePathString.c_str(), state.createModelPathBuffer.size() - 1);
    }
}
