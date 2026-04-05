#include "editor/EditorCreationPanel.h"

#include "core/EventBus.h"
#include "core/Events.h"
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
        if (!state.eventBus) return;  // Safety check

        const std::string name = state.createNameBuffer[0] != '\0' ? state.createNameBuffer.data() : "NewObject";
        const std::string modelPath = state.createModelPathBuffer.data();
        const std::string texturePath = state.createTexturePathBuffer.data();

        OGLE::EditorCreateEntityEvent event;
        event.name = name;
        event.modelPath = modelPath;
        event.texturePath = texturePath;

        switch (state.createKind) {
            case 0: event.type = OGLE::EditorCreateEntityEvent::Type::EmptyObject; break;
            case 1: event.type = OGLE::EditorCreateEntityEvent::Type::Cube; break;
            case 2: event.type = OGLE::EditorCreateEntityEvent::Type::Sphere; break;
            case 3: event.type = OGLE::EditorCreateEntityEvent::Type::Plane; break;
            case 5: event.type = OGLE::EditorCreateEntityEvent::Type::DirectionalLight; break;
            case 6: event.type = OGLE::EditorCreateEntityEvent::Type::PointLight; break;
            case 4: event.type = OGLE::EditorCreateEntityEvent::Type::ModelFromFile; break;
            default: return;
        }

        state.eventBus->Dispatch(event);
    }
}
