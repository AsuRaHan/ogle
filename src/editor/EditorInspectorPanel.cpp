#include "editor/EditorInspectorPanel.h"

#include <imgui.h>
#include "ImGuizmo.h"
#include "editor/EditorAssetHelpers.h"
#include "editor/EditorState.h"
#include "managers/CameraManager.h"
#include "managers/PhysicsManager.h"
#include "managers/WorldManager.h"
#include "opengl/Camera.h"
#include "opengl/ShaderManager.h"
#include "render/MaterialLibrary.h"
#include "render/AnimationLibrary.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

namespace
{
    static ImGuizmo::OPERATION m_currentGizmoOperation(ImGuizmo::TRANSLATE);

    const char* GetPrimitiveTypeLabel(OGLE::PrimitiveType type)
    {
        switch (type) {
            case OGLE::PrimitiveType::None: return "None";
            case OGLE::PrimitiveType::Cube: return "Cube";
            case OGLE::PrimitiveType::Sphere: return "Sphere";
            case OGLE::PrimitiveType::Plane: return "Plane";
            case OGLE::PrimitiveType::ModelFile: return "Model File";
            case OGLE::PrimitiveType::ProceduralMesh: return "Procedural Mesh";
            default: return "Unknown";
        }
    }
}

void EditorInspectorPanel::Draw(EditorState& state, WorldManager& worldManager, PhysicsManager& physicsManager, const CameraManager& cameraManager)
{
    if (state.selectedEntity == entt::null || !worldManager.IsEntityValid(state.selectedEntity)) {
        ImGui::TextUnformatted("No object selected.");
        return;
    }

    auto selectedObject = worldManager.GetWorldObject(state.selectedEntity);
    OGLE::World& world = worldManager.GetActiveWorld();
    OGLE::WorldObjectComponent* worldObject = world.GetWorldObjectComponent(state.selectedEntity);
    OGLE::TransformComponent currentTransform = selectedObject.GetTransform();
    glm::vec3 position = currentTransform.position;
    glm::vec3 rotation = currentTransform.rotation;
    glm::vec3 scale = currentTransform.scale;

    ImGui::Text("Entity: %u", static_cast<unsigned int>(entt::to_integral(state.selectedEntity)));
    ImGui::InputText("Name", state.selectedNameBuffer.data(), state.selectedNameBuffer.size());
    if (ImGui::Button("Apply Name")) {
        selectedObject.SetName(state.selectedNameBuffer.data());
    }

    if (worldObject) {
        bool enabled = worldObject->enabled;
        bool visible = worldObject->visible;
        int kindIndex = static_cast<int>(worldObject->kind);

        if (ImGui::Checkbox("Enabled", &enabled)) {
            worldObject->enabled = enabled;
        }
        if (ImGui::Checkbox("Visible", &visible)) {
            worldObject->visible = visible;
        }

        const char* kindLabels[] = { "Generic", "Mesh", "Light", "Billboard" };
        if (ImGui::Combo("Kind", &kindIndex, kindLabels, IM_ARRAYSIZE(kindLabels))) {
            worldObject->kind = static_cast<OGLE::WorldObjectKind>(kindIndex);
        }
    }

    bool transformChanged = false;
    transformChanged |= ImGui::DragFloat3("Position", glm::value_ptr(position), 0.05f);
    transformChanged |= ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.5f);
    transformChanged |= ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.05f, 0.01f, 1000.0f);
    if (transformChanged) {
        selectedObject.SetTransform(position, rotation, scale);
    }

    // ImGuizmo
    if (ImGui::RadioButton("Translate", m_currentGizmoOperation == ImGuizmo::TRANSLATE))
        m_currentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", m_currentGizmoOperation == ImGuizmo::ROTATE))
        m_currentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", m_currentGizmoOperation == ImGuizmo::SCALE))
        m_currentGizmoOperation = ImGuizmo::SCALE;

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetRect(ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y, ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->Size.y);

    const auto& camera = cameraManager.GetCamera();
    const glm::mat4& viewMatrix = camera.GetViewMatrix();
    const glm::mat4& projectionMatrix = camera.GetProjectionMatrix();

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, scale);

    ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix), m_currentGizmoOperation, ImGuizmo::LOCAL, glm::value_ptr(modelMatrix));

    if (ImGuizmo::IsUsing())
    {
        glm::vec3 newPosition, newScale, newRotation;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), glm::value_ptr(newPosition), glm::value_ptr(newRotation), glm::value_ptr(newScale));
        selectedObject.SetTransform(newPosition, newRotation, newScale);
    }


    if (OGLE::PrimitiveComponent* primitive = world.GetPrimitive(state.selectedEntity)) {
        if (ImGui::CollapsingHeader("Primitive", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("Type: %s", GetPrimitiveTypeLabel(primitive->type));
            ImGui::InputText("Source Path", state.primitiveSourcePathBuffer.data(), state.primitiveSourcePathBuffer.size());
            ImGui::TextDisabled("Primitive describes how this entity was created.");
            if (ImGui::Button("Apply Primitive")) {
                primitive->sourcePath = state.primitiveSourcePathBuffer.data();
            }
        }
    }

    if (OGLE::ModelEntity* model = selectedObject.GetModel()) {
        if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("Model Source: %s", model->GetFilePath().empty() ? "<procedural>" : model->GetFilePath().c_str());
            ImGui::TextDisabled("Mesh data stays on ModelEntity, while authoring data is split into components below.");
        }
    }

    if (OGLE::MaterialComponent* materialComponent = world.GetMaterial(state.selectedEntity)) {
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            // The editable material lives in MaterialComponent so the inspector can treat it as a standalone primitive.
            ImGui::ColorEdit3("Base Color", glm::value_ptr(state.baseColorBuffer));
            ImGui::ColorEdit3("Emissive Color", glm::value_ptr(state.emissiveColorBuffer));
            ImGui::DragFloat2("UV Tiling", glm::value_ptr(state.uvTilingBuffer), 0.01f, -100.0f, 100.0f);
            ImGui::DragFloat2("UV Offset", glm::value_ptr(state.uvOffsetBuffer), 0.01f, -100.0f, 100.0f);
            ImGui::SliderFloat("Roughness", &state.roughnessBuffer, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &state.metallicBuffer, 0.0f, 1.0f);
            ImGui::SliderFloat("Alpha Cutoff", &state.alphaCutoffBuffer, 0.0f, 1.0f);
            ImGui::InputText("Diffuse Texture", state.texturePathBuffer.data(), state.texturePathBuffer.size());
            ImGui::InputText("Emissive Texture", state.emissiveTexturePathBuffer.data(), state.emissiveTexturePathBuffer.size());

            // Shader selection combo: loaded shader programs from ShaderManager
            ShaderManager* shaderManager = ShaderManager::GetGlobalInstance();
            std::vector<std::string> programNames = shaderManager ? shaderManager->GetProgramNames() : std::vector<std::string>{"default"};
            if (programNames.empty()) {
                programNames.push_back("default");
            }

            // Ensure current buffer has some valid content
            if (state.shaderProgramBuffer[0] == '\0' && !programNames.empty()) {
                std::strncpy(state.shaderProgramBuffer.data(), programNames[0].c_str(), state.shaderProgramBuffer.size() - 1);
            }

            if (ImGui::BeginCombo("Shader Program", state.shaderProgramBuffer.data())) {
                for (const std::string& programName : programNames) {
                    bool selected = std::strcmp(state.shaderProgramBuffer.data(), programName.c_str()) == 0;
                    if (ImGui::Selectable(programName.c_str(), selected)) {
                        std::strncpy(state.shaderProgramBuffer.data(), programName.c_str(), state.shaderProgramBuffer.size() - 1);
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Apply Material")) {
                materialComponent->material.SetBaseColor(state.baseColorBuffer);
                materialComponent->material.SetEmissiveColor(state.emissiveColorBuffer);
                materialComponent->material.SetUvTiling(state.uvTilingBuffer);
                materialComponent->material.SetUvOffset(state.uvOffsetBuffer);
                materialComponent->material.SetRoughness(state.roughnessBuffer);
                materialComponent->material.SetMetallic(state.metallicBuffer);
                materialComponent->material.SetAlphaCutoff(state.alphaCutoffBuffer);
                materialComponent->material.SetDiffuseTexturePath(state.texturePathBuffer.data());
                materialComponent->material.SetEmissiveTexturePath(state.emissiveTexturePathBuffer.data());
                materialComponent->material.SetShaderProgram(state.shaderProgramBuffer.data());

                // Apply new or existing material asset name if it exists
                if (OGLE::Material* assetMaterial = OGLE::MaterialLibrary::Instance().GetMaterial(state.shaderProgramBuffer.data())) {
                    materialComponent->material = *assetMaterial;
                }

                // RHS: if scene has shader component, sync too
                OGLE::ShaderComponent* shaderComp = world.GetShader(state.selectedEntity);
                if (!shaderComp) {
                    world.GetRegistry().emplace<OGLE::ShaderComponent>(state.selectedEntity, OGLE::ShaderComponent{std::string(state.shaderProgramBuffer.data())});
                } else {
                    shaderComp->programName = state.shaderProgramBuffer.data();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Save As Material Asset")) {
                const std::string assetName = std::string(state.shaderProgramBuffer.data());
                OGLE::MaterialLibrary::Instance().AddMaterial(assetName, materialComponent->material);
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear Material Asset")) {
                OGLE::MaterialLibrary::Instance().RemoveMaterial(state.shaderProgramBuffer.data());
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear Textures")) {
                materialComponent->material.SetDiffuseTexturePath("");
                materialComponent->material.SetEmissiveTexturePath("");
                state.texturePathBuffer.fill('\0');
                state.emissiveTexturePathBuffer.fill('\0');
            }

            ImGui::InputText("Material Library Path", state.assetsPathBuffer.data(), state.assetsPathBuffer.size());
            ImGui::SameLine();
            if (ImGui::Button("Save Materials")) {
                OGLE::MaterialLibrary::Instance().SaveToFile(state.assetsPathBuffer.data());
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Materials")) {
                OGLE::MaterialLibrary::Instance().LoadFromFile(state.assetsPathBuffer.data());
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GetContentBrowserAssetPayload())) {
                    const char* assetPath = static_cast<const char*>(payload->Data);
                    if (assetPath && IsEditorTextureAssetPath(assetPath)) {
                        materialComponent->material.SetDiffuseTexturePath(assetPath);
                        state.texturePathBuffer.fill('\0');
                        std::strncpy(state.texturePathBuffer.data(), assetPath, state.texturePathBuffer.size() - 1);
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
    }

    if (OGLE::LightComponent* light = world.GetLight(state.selectedEntity)) {
        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            int lightType = light->type == OGLE::LightType::Directional ? 0 : 1;
            const char* lightTypeLabels[] = { "Directional", "Point" };
            ImGui::Combo("Light Type", &lightType, lightTypeLabels, IM_ARRAYSIZE(lightTypeLabels));

            glm::vec3 lightColor = light->color;
            float lightIntensity = light->intensity;
            float lightRange = light->range;
            bool castShadows = light->castShadows;
            bool primary = light->primary;

            ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));
            ImGui::DragFloat("Light Intensity", &lightIntensity, 0.05f, 0.0f, 100.0f);
            if (lightType == 1) {
                ImGui::DragFloat("Light Range", &lightRange, 0.1f, 0.1f, 1000.0f);
            } else {
                ImGui::Checkbox("Cast Shadows", &castShadows);
                ImGui::Checkbox("Primary Light", &primary);
                ImGui::TextDisabled("Directional light direction comes from object rotation.");
            }

            if (ImGui::Button("Apply Light")) {
                light->type = lightType == 0 ? OGLE::LightType::Directional : OGLE::LightType::Point;
                light->color = lightColor;
                light->intensity = lightIntensity;
                light->range = lightRange;
                light->castShadows = light->type == OGLE::LightType::Directional ? castShadows : false;
                light->primary = light->type == OGLE::LightType::Directional ? primary : false;
            }
        }
    }

    OGLE::PhysicsBodyComponent* physicsBody = world.GetPhysicsBody(state.selectedEntity);
    if (physicsBody) {
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            int bodyType = static_cast<int>(physicsBody->type);
            int shapeType = static_cast<int>(physicsBody->shape);
            float mass = physicsBody->mass;
            glm::vec3 halfExtents = physicsBody->halfExtents;
            float radius = physicsBody->radius;
            float height = physicsBody->height;
            bool simulate = physicsBody->simulate;
            bool isTrigger = physicsBody->isTrigger;
            float friction = physicsBody->friction;
            float restitution = physicsBody->restitution;

            const char* bodyTypeLabels[] = { "Static", "Dynamic", "Kinematic" };
            const char* shapeLabels[] = { "Box", "Sphere", "Capsule" };

            ImGui::Combo("Body Type", &bodyType, bodyTypeLabels, IM_ARRAYSIZE(bodyTypeLabels));
            ImGui::Combo("Shape", &shapeType, shapeLabels, IM_ARRAYSIZE(shapeLabels));
            ImGui::DragFloat("Mass", &mass, 0.1f, 0.0f, 1000.0f);
            if (shapeType == static_cast<int>(OGLE::PhysicsShapeType::Box)) {
                ImGui::DragFloat3("Half Extents", glm::value_ptr(halfExtents), 0.05f, 0.01f, 1000.0f);
            } else if (shapeType == static_cast<int>(OGLE::PhysicsShapeType::Sphere)) {
                ImGui::DragFloat("Radius", &radius, 0.01f, 0.01f, 1000.0f);
            } else if (shapeType == static_cast<int>(OGLE::PhysicsShapeType::Capsule)) {
                ImGui::DragFloat("Radius", &radius, 0.01f, 0.01f, 1000.0f);
                ImGui::DragFloat("Height", &height, 0.01f, 0.01f, 1000.0f);
            }
            ImGui::Checkbox("Simulate", &simulate);
            ImGui::Checkbox("Is Trigger", &isTrigger);
            ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f);

            if (ImGui::Button("Apply Physics")) {
                physicsBody->type = static_cast<OGLE::PhysicsBodyType>(bodyType);
                physicsBody->shape = static_cast<OGLE::PhysicsShapeType>(shapeType);
                physicsBody->mass = mass;
                physicsBody->halfExtents = halfExtents;
                physicsBody->radius = radius;
                physicsBody->height = height;
                physicsBody->simulate = simulate;
                physicsBody->isTrigger = isTrigger;
                physicsBody->friction = friction;
                physicsBody->restitution = restitution;

                if (simulate) {
                    if (physicsBody->shape == OGLE::PhysicsShapeType::Box) {
                        physicsManager.AddBoxBody(state.selectedEntity, halfExtents, physicsBody->type, mass);
                    } else if (physicsBody->shape == OGLE::PhysicsShapeType::Sphere) {
                        physicsManager.AddSphereBody(state.selectedEntity, radius, physicsBody->type, mass);
                    } else if (physicsBody->shape == OGLE::PhysicsShapeType::Capsule) {
                        physicsManager.AddCapsuleBody(state.selectedEntity, radius, height, physicsBody->type, mass);
                    }
                } else {
                    physicsManager.RemoveBody(state.selectedEntity);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Physics")) {
                physicsManager.RemoveBody(state.selectedEntity);
                world.GetRegistry().remove<OGLE::PhysicsBodyComponent>(state.selectedEntity);
            }
        }
    }

    if (OGLE::SkeletonComponent* skeleton = world.GetSkeleton(state.selectedEntity)) {
        if (ImGui::CollapsingHeader("Skeleton", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Skeleton Enabled", &skeleton->enabled);
            ImGui::InputInt("Bone Count", &skeleton->boneCount);
            ImGui::InputText("Skeleton Source", state.skeletonSourcePathBuffer.data(), state.skeletonSourcePathBuffer.size());
            ImGui::TextDisabled("This is the authoring hook for future skinned mesh support.");
            if (ImGui::Button("Apply Skeleton")) {
                skeleton->sourcePath = state.skeletonSourcePathBuffer.data();
                if (skeleton->boneCount < 0) {
                    skeleton->boneCount = 0;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Skeleton")) {
                world.GetRegistry().remove<OGLE::SkeletonComponent>(state.selectedEntity);
                state.skeletonSourcePathBuffer.fill('\0');
            }
        }
    }

    if (OGLE::AnimationComponent* animation = world.GetAnimation(state.selectedEntity)) {
        if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Animation Enabled", &animation->enabled);
            ImGui::Checkbox("Playing", &animation->playing);
            ImGui::Checkbox("Loop", &animation->loop);
            ImGui::DragFloat("Current Time", &animation->currentTime, 0.01f, 0.0f, 3600.0f);
            ImGui::DragFloat("Playback Speed", &animation->playbackSpeed, 0.01f, 0.0f, 10.0f);
            ImGui::InputText("Clip", state.animationClipBuffer.data(), state.animationClipBuffer.size());
            if (ImGui::Button("Apply Animation")) {
                animation->currentClip = state.animationClipBuffer.data();
                if (animation->playbackSpeed < 0.0f) {
                    animation->playbackSpeed = 0.0f;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Save As Animation Asset")) {
                const std::string assetName = std::string(state.animationClipBuffer.data());
                OGLE::AnimationLibrary::Instance().AddAnimation(assetName, *animation);
            }

            ImGui::SameLine();
            if (ImGui::Button("Apply Animation Asset")) {
                const std::string assetName = std::string(state.animationClipBuffer.data());
                if (OGLE::AnimationComponent* asset = OGLE::AnimationLibrary::Instance().GetAnimation(assetName)) {
                    *animation = *asset;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Remove Animation")) {
                world.GetRegistry().remove<OGLE::AnimationComponent>(state.selectedEntity);
                state.animationClipBuffer.fill('\0');
            }

            ImGui::InputText("Animation Library Path", state.assetsPathBuffer.data(), state.assetsPathBuffer.size());
            ImGui::SameLine();
            if (ImGui::Button("Save Animations")) {
                OGLE::AnimationLibrary::Instance().SaveToFile(state.assetsPathBuffer.data());
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Animations")) {
                OGLE::AnimationLibrary::Instance().LoadFromFile(state.assetsPathBuffer.data());
            }
        }
    }

    if (OGLE::ScriptComponent* script = world.GetScript(state.selectedEntity)) {
        if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Script Enabled", &script->enabled);
            ImGui::Checkbox("Auto Start", &script->autoStart);
            ImGui::InputText("Script Path", state.scriptPathBuffer.data(), state.scriptPathBuffer.size());
            ImGui::TextDisabled("ScriptComponent stores which JS file belongs to this entity.");
            if (ImGui::Button("Apply Script")) {
                script->scriptPath = state.scriptPathBuffer.data();
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Script")) {
                world.GetRegistry().remove<OGLE::ScriptComponent>(state.selectedEntity);
                state.scriptPathBuffer.fill('\0');
            }
        }
    }

    if (ImGui::CollapsingHeader("Add Components", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("Build entities from small editable primitives.");

        if (!world.GetMaterial(state.selectedEntity)) {
            if (ImGui::Button("Add Material")) {
                world.GetRegistry().emplace<OGLE::MaterialComponent>(state.selectedEntity);
                state.bufferedEntity = entt::null;
            }
        }

        if (!world.GetPhysicsBody(state.selectedEntity)) {
            if (ImGui::Button("Add Physics")) {
                world.GetRegistry().emplace<OGLE::PhysicsBodyComponent>(state.selectedEntity);
            }
        }

        if (!world.GetLight(state.selectedEntity)) {
            if (ImGui::Button("Add Light")) {
                world.GetRegistry().emplace<OGLE::LightComponent>(state.selectedEntity);
                if (worldObject) {
                    worldObject->kind = OGLE::WorldObjectKind::Light;
                }
            }
        }

        if (!world.GetSkeleton(state.selectedEntity)) {
            if (ImGui::Button("Add Skeleton")) {
                world.GetRegistry().emplace<OGLE::SkeletonComponent>(state.selectedEntity);
                state.bufferedEntity = entt::null;
            }
        }

        if (!world.GetAnimation(state.selectedEntity)) {
            if (ImGui::Button("Add Animation")) {
                world.GetRegistry().emplace<OGLE::AnimationComponent>(state.selectedEntity);
                state.bufferedEntity = entt::null;
            }
        }

        if (!world.GetScript(state.selectedEntity)) {
            if (ImGui::Button("Add Script")) {
                world.GetRegistry().emplace<OGLE::ScriptComponent>(state.selectedEntity);
                state.bufferedEntity = entt::null;
            }
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Delete Selected")) {
        physicsManager.RemoveBody(state.selectedEntity);
        world.DestroyEntity(state.selectedEntity);
        state.selectedEntity = entt::null;
        state.bufferedEntity = entt::null;
        state.textureEditingEntity = entt::null;
    }
}
