#include "editor/EditorAnimationPanel.h"
#include "render/AnimationLibrary.h"
#include "world/WorldComponents.h"
#include <imgui.h>
#include <cstring>
#include <algorithm>

void EditorAnimationPanel::Draw(EditorState& state, WorldManager& worldManager)
{
    OGLE::World& world = worldManager.GetActiveWorld();

    if (state.selectedEntity == entt::null || !worldManager.IsEntityValid(state.selectedEntity)) {
        ImGui::TextUnformatted("No entity selected.");
        return;
    }

    ImGui::Text("Entity: %u", static_cast<unsigned int>(entt::to_integral(state.selectedEntity)));

    OGLE::AnimationComponent* animation = world.GetAnimation(state.selectedEntity);
    if (!animation) {
        ImGui::TextDisabled("No animation component.");
        if (ImGui::Button("Add Animation Component")) {
            world.GetRegistry().emplace<OGLE::AnimationComponent>(state.selectedEntity);
            animation = world.GetAnimation(state.selectedEntity);
        }
        return;
    }

    ImGui::Checkbox("Enabled", &animation->enabled);
    ImGui::Checkbox("Playing", &animation->playing);
    ImGui::Checkbox("Loop", &animation->loop);
    float clipDuration = std::max(0.001f, animation->duration);
    ImGui::DragFloat("Duration", &animation->duration, 0.01f, 0.01f, 3600.0f);

    float maxTime = animation->duration > 0.0f ? animation->duration : 3600.0f;
    animation->currentTime = std::clamp(animation->currentTime, 0.0f, maxTime);

    ImGui::SliderFloat("Timeline", &animation->currentTime, 0.0f, maxTime);
    ImGui::DragFloat("Playback Speed", &animation->playbackSpeed, 0.01f, 0.0f, 10.0f);
    ImGui::InputText("Current Clip", state.animationClipBuffer.data(), state.animationClipBuffer.size());

    if (ImGui::Button(animation->playing ? "Pause" : "Play")) {
        animation->playing = !animation->playing;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        animation->playing = false;
        animation->currentTime = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply Clip")) {
        animation->currentClip = state.animationClipBuffer.data();
        for (int i = 0; i < static_cast<int>(animation->clips.size()); ++i) {
            if (animation->clips[i].name == animation->currentClip) {
                animation->currentClipIndex = i;
                animation->duration = animation->clips[i].duration;
                break;
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Animation")) {
        world.GetRegistry().remove<OGLE::AnimationComponent>(state.selectedEntity);
        state.animationClipBuffer.fill('\0');
        return;
    }

    if (!animation->clips.empty()) {
        ImGui::Separator();
        ImGui::Text("Imported Clips (%zu)", animation->clips.size());

        static int selectedClipIndex = 0;
        selectedClipIndex = std::clamp(selectedClipIndex, 0, static_cast<int>(animation->clips.size()) - 1);

        if (ImGui::BeginCombo("Select Clip", animation->clips[selectedClipIndex].name.c_str())) {
            for (int i = 0; i < static_cast<int>(animation->clips.size()); ++i) {
                const bool isSelected = (i == selectedClipIndex);
                if (ImGui::Selectable(animation->clips[i].name.c_str(), isSelected)) {
                    selectedClipIndex = i;
                    animation->currentClipIndex = i;
                    animation->currentClip = animation->clips[i].name;
                    animation->duration = animation->clips[i].duration;
                    animation->currentTime = 0.0f;
                    std::strncpy(state.animationClipBuffer.data(), animation->currentClip.c_str(), state.animationClipBuffer.size() - 1);
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        const auto& clip = animation->clips[selectedClipIndex];
        ImGui::Text("Clip duration: %.3f s, tracks: %zu", clip.duration, clip.tracks.size());

        for (const auto& track : clip.tracks) {
            ImGui::Text("Track: %s (%zu keys)", track.nodeName.c_str(), track.keyframes.size());
            ImVec2 drawPos = ImGui::GetCursorScreenPos();
            ImVec2 drawSize = ImVec2(360.0f, 18.0f);
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(drawPos, ImVec2(drawPos.x + drawSize.x, drawPos.y + drawSize.y), IM_COL32(40, 40, 40, 200));
            for (const auto& key : track.keyframes) {
                float norm = (clip.duration > 0.0f) ? (key.time / clip.duration) : 0.0f;
                float x = drawPos.x + std::clamp(norm, 0.0f, 1.0f) * drawSize.x;
                drawList->AddLine(ImVec2(x, drawPos.y), ImVec2(x, drawPos.y + drawSize.y), IM_COL32(255, 180, 50, 255), 2.0f);
            }
            ImGui::Dummy(drawSize);
        }
    }

    ImGui::Separator();
    ImGui::Text("Animation Assets (%zu)", OGLE::AnimationLibrary::Instance().GetAnimationNames().size());

    static char selectedAsset[256] = "";
    auto assetNames = OGLE::AnimationLibrary::Instance().GetAnimationNames();
    if (ImGui::BeginCombo("Asset to Apply", selectedAsset[0] ? selectedAsset : "(None)")) {
        for (const auto& nm : assetNames) {
            bool isSelected = std::strcmp(nm.c_str(), selectedAsset) == 0;
            if (ImGui::Selectable(nm.c_str(), isSelected)) {
                std::strncpy(selectedAsset, nm.c_str(), sizeof(selectedAsset) - 1);
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Apply Asset") && selectedAsset[0] != '\0') {
        if (auto asset = OGLE::AnimationLibrary::Instance().GetAnimation(selectedAsset)) {
            *animation = *asset;
            std::strncpy(state.animationClipBuffer.data(), animation->currentClip.c_str(), state.animationClipBuffer.size() - 1);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Asset")) {
        std::string name = state.animationClipBuffer.data();
        if (name.empty()) {
            name = "clip_" + std::to_string(static_cast<int>(animation->currentTime));
        }
        OGLE::AnimationLibrary::Instance().AddAnimation(name, *animation);
    }

    ImGui::Separator();
    ImGui::InputText("Animation Library Path", state.assetsPathBuffer.data(), state.assetsPathBuffer.size());

    if (ImGui::Button("Save Animation Library")) {
        OGLE::AnimationLibrary::Instance().SaveToFile(state.assetsPathBuffer.data());
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Animation Library")) {
        OGLE::AnimationLibrary::Instance().LoadFromFile(state.assetsPathBuffer.data());
    }
}
