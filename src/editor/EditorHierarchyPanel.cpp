#include "editor/EditorHierarchyPanel.h"

#include "core/EventBus.h"
#include "core/Events.h"
#include "editor/EditorState.h"
#include "managers/PhysicsManager.h"
#include "managers/WorldManager.h"

#include <imgui.h>
#include <algorithm>
#include <vector>

namespace
{
    const char* GetKindLabel(OGLE::WorldObjectKind kind)
    {
        switch (kind) {
            case OGLE::WorldObjectKind::Generic: return "Generic";
            case OGLE::WorldObjectKind::Mesh: return "Mesh";
            case OGLE::WorldObjectKind::Light: return "Light";
            case OGLE::WorldObjectKind::Billboard: return "Billboard";
            default: return "Unknown";
        }
    }
}

void EditorHierarchyPanel::Draw(EditorState& state, WorldManager& worldManager, PhysicsManager& physicsManager)
{
    auto nameView = worldManager.GetActiveWorld().GetRegistry().view<OGLE::NameComponent, OGLE::WorldObjectComponent>();
    std::vector<OGLE::Entity> entities;
    for (auto entity : nameView) {
        entities.push_back(entity);
    }

    std::sort(entities.begin(), entities.end(), [&nameView](OGLE::Entity a, OGLE::Entity b) {
        return nameView.get<OGLE::NameComponent>(a).value < nameView.get<OGLE::NameComponent>(b).value;
    });

    if (ImGui::Button("Add Empty")) {
        if (state.eventBus) {
            state.eventBus->Dispatch(OGLE::EditorCreateEntityEvent{
                OGLE::EditorCreateEntityEvent::Type::EmptyObject,
                "EmptyObject",
                "",
                ""
            });
        }
    }
    // ImGui::SameLine();
    // if (ImGui::Button("Add Cube")) {
    //     state.selectedEntity = worldManager.CreateCube("Cube", glm::vec3(0.0f, 0.5f, 0.0f));
    //     state.bufferedEntity = entt::null;
    // }
    ImGui::SameLine();
    const bool canDelete = state.selectedEntity != entt::null && worldManager.IsEntityValid(state.selectedEntity);
    if (!canDelete) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Delete")) {
        if (state.eventBus) {
            state.eventBus->Dispatch(OGLE::EditorDeleteEntityEvent{ state.selectedEntity });
        }
    }
    if (!canDelete) {
        ImGui::EndDisabled();
    }

    ImGui::Separator();
    const bool opened = ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen);
    if (opened) {
        if (entities.empty()) {
            ImGui::TextDisabled("Empty");
        } else {
            for (OGLE::Entity entity : entities) {
                const auto& name = nameView.get<OGLE::NameComponent>(entity);
                const auto& object = nameView.get<OGLE::WorldObjectComponent>(entity);
                const bool selected = entity == state.selectedEntity;
                ImGuiTreeNodeFlags itemFlags =
                    ImGuiTreeNodeFlags_Leaf |
                    ImGuiTreeNodeFlags_NoTreePushOnOpen |
                    (selected ? ImGuiTreeNodeFlags_Selected : 0);

                ImGui::TreeNodeEx(
                    reinterpret_cast<void*>(static_cast<intptr_t>(entt::to_integral(entity))),
                    itemFlags,
                    "%s [%s]##%u",
                    name.value.c_str(),
                    GetKindLabel(object.kind),
                    static_cast<unsigned int>(entt::to_integral(entity)));

                if (ImGui::IsItemClicked()) {
                    state.selectedEntity = entity;
                }

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) {
                        if (state.eventBus) {
                            state.eventBus->Dispatch(OGLE::EditorDeleteEntityEvent{ entity });
                        }
                        ImGui::EndPopup();
                        break;
                    }
                    ImGui::EndPopup();
                }
            }
        }
        ImGui::TreePop();
    }
}
