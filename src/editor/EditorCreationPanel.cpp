#include "editor/EditorCreationPanel.h"

#include "editor/EditorState.h"
#include "managers/WorldManager.h"

#include <imgui.h>

void EditorCreationPanel::Draw(EditorState& state, WorldManager& worldManager)
{
    ImGui::TextUnformatted("Create");
    ImGui::Separator();

    const char* createKinds[] = { "Empty Object", "Cube", "Sphere", "Plane", "Model From File", "Directional Light", "Point Light" };
    ImGui::Combo("Create Type", &state.createKind, createKinds, IM_ARRAYSIZE(createKinds));
    ImGui::InputText("New Name", state.createNameBuffer.data(), state.createNameBuffer.size());

    if (state.createKind == 4) {
        ImGui::InputText("Model Path", state.createModelPathBuffer.data(), state.createModelPathBuffer.size());
    }

    if (state.createKind == 1 || state.createKind == 2 || state.createKind == 3 || state.createKind == 4) {
        ImGui::InputText("Texture Path", state.createTexturePathBuffer.data(), state.createTexturePathBuffer.size());
    }

    if (ImGui::Button("Create")) {
        const std::string name = state.createNameBuffer[0] != '\0' ? state.createNameBuffer.data() : "NewObject";

        if (state.createKind == 0) {
            state.selectedEntity = worldManager.CreateWorldObject(name, OGLE::WorldObjectKind::Generic).GetEntity();
        } else if (state.createKind == 1) {
            state.selectedEntity = worldManager.CreatePrimitive(
                name,
                OGLE::PrimitiveType::Cube,
                glm::vec3(0.0f, 0.5f, 0.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                state.createTexturePathBuffer.data());
        } else if (state.createKind == 2) {
            state.selectedEntity = worldManager.CreatePrimitive(
                name,
                OGLE::PrimitiveType::Sphere,
                glm::vec3(0.0f, 0.5f, 0.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                state.createTexturePathBuffer.data());
        } else if (state.createKind == 3) {
            state.selectedEntity = worldManager.CreatePrimitive(
                name,
                OGLE::PrimitiveType::Plane,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(5.0f, 1.0f, 5.0f),
                state.createTexturePathBuffer.data());
        } else if (state.createKind == 5) {
            state.selectedEntity = worldManager.CreateDirectionalLight(
                name,
                glm::vec3(-50.0f, 45.0f, 0.0f));
        } else if (state.createKind == 6) {
            state.selectedEntity = worldManager.CreatePointLight(
                name,
                glm::vec3(0.0f, 1.5f, 0.0f));
        } else {
            state.selectedEntity = worldManager.CreateModelFromFile(
                state.createModelPathBuffer.data(),
                OGLE::ModelType::DYNAMIC,
                name);

            if (state.selectedEntity != entt::null && state.createTexturePathBuffer[0] != '\0') {
                worldManager.SetEntityDiffuseTexture(state.selectedEntity, state.createTexturePathBuffer.data());
            }
        }

        state.bufferedEntity = entt::null;
        state.textureEditingEntity = entt::null;
    }
}
