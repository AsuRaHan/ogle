#include "editor/Editor.h"

#include "Logger.h"
#include "editor/EditorAssetHelpers.h"
#include "input/InputController.h"
#include "managers/CameraManager.h"
#include "managers/PhysicsManager.h"
#include "managers/WorldManager.h"
#include "opengl/Camera.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <imgui.h>
#include <cstring>
#include <limits>
#include <utility>

bool Editor::Initialize()
{
    if (m_initialized) {
        return true;
    }

    m_worldPathBuffer.fill('\0');
    m_assetsPathBuffer.fill('\0');
    m_selectedNameBuffer.fill('\0');
    m_createNameBuffer.fill('\0');
    m_createModelPathBuffer.fill('\0');
    m_createTexturePathBuffer.fill('\0');
    m_contentSelectionBuffer.fill('\0');
    m_primitiveSourcePathBuffer.fill('\0');
    m_skeletonSourcePathBuffer.fill('\0');
    m_animationClipBuffer.fill('\0');
    m_scriptPathBuffer.fill('\0');
    m_texturePathBuffer.fill('\0');
    m_emissiveTexturePathBuffer.fill('\0');
    std::strncpy(m_createNameBuffer.data(), "NewObject", m_createNameBuffer.size() - 1);
    std::strncpy(m_worldPathBuffer.data(), "assets/worlds/default_world.json", m_worldPathBuffer.size() - 1);
    std::strncpy(m_assetsPathBuffer.data(), "assets", m_assetsPathBuffer.size() - 1);

    m_initialized = true;
    LOG_INFO("Editor initialized");
    return true;
}

void Editor::Shutdown()
{
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    LOG_INFO("Editor shutdown");
}

void Editor::SetEnabled(bool enabled)
{
    m_enabled = enabled;
}

void Editor::Toggle()
{
    m_enabled = !m_enabled;
}

bool Editor::IsEnabled() const
{
    return m_enabled;
}

bool Editor::IsInitialized() const
{
    return m_initialized;
}

OGLE::Entity Editor::GetSelectedEntity() const
{
    return m_selectedEntity;
}

Editor::SimulationState Editor::GetSimulationState() const
{
    return m_simulationState;
}

bool Editor::ConsumeSimulationStepRequest()
{
    const bool shouldStep = m_stepSimulationRequested;
    m_stepSimulationRequested = false;
    return shouldStep;
}

void Editor::BuildUi(
    const CameraManager& cameraManager,
    WorldManager& worldManager,
    PhysicsManager& physicsManager,
    ConfigManager& configManager)
{
    if (!m_initialized || !m_enabled) {
        return;
    }

    const ogle::Camera& camera = cameraManager.GetCamera();
    if (!ImGui::GetIO().WantCaptureMouse) {
        TrySelectObject(camera, worldManager);
    }

    if (m_selectedEntity != entt::null && !worldManager.IsEntityValid(m_selectedEntity)) {
        m_selectedEntity = entt::null;
        m_bufferedEntity = entt::null;
        m_textureEditingEntity = entt::null;
    }

    if (m_worldPathBuffer[0] == '\0') {
        std::strncpy(
            m_worldPathBuffer.data(),
            configManager.GetConfig().world.path.c_str(),
            m_worldPathBuffer.size() - 1);
    }
    if (m_assetsPathBuffer[0] == '\0') {
        std::strncpy(
            m_assetsPathBuffer.data(),
            configManager.GetConfig().assets.path.c_str(),
            m_assetsPathBuffer.size() - 1);
    }

    SyncSelectedBuffers(worldManager);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load World")) {
                configManager.GetConfig().world.path = m_worldPathBuffer.data();
                configManager.Save();
                worldManager.LoadActiveWorld(m_worldPathBuffer.data());
                m_selectedEntity = entt::null;
                m_bufferedEntity = entt::null;
                m_textureEditingEntity = entt::null;
            }
            if (ImGui::MenuItem("Save World")) {
                configManager.GetConfig().world.path = m_worldPathBuffer.data();
                configManager.Save();
                worldManager.SaveActiveWorld(m_worldPathBuffer.data());
            }
            if (ImGui::MenuItem("Reload Default")) {
                worldManager.CreateDefaultWorld();
                m_selectedEntity = entt::null;
                m_bufferedEntity = entt::null;
                m_textureEditingEntity = entt::null;
            }
            if (ImGui::MenuItem("Clear World")) {
                worldManager.ClearWorld();
                m_selectedEntity = entt::null;
                m_bufferedEntity = entt::null;
                m_textureEditingEntity = entt::null;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Simulation")) {
            const bool isPlaying = m_simulationState == SimulationState::Playing;
            if (ImGui::MenuItem("Play", nullptr, isPlaying)) {
                m_simulationState = SimulationState::Playing;
            }
            if (ImGui::MenuItem("Pause", nullptr, !isPlaying)) {
                m_simulationState = SimulationState::Paused;
            }
            if (ImGui::MenuItem("Step")) {
                m_simulationState = SimulationState::Paused;
                m_stepSimulationRequested = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("World", nullptr, &m_showWorldWindow);
            ImGui::MenuItem("Hierarchy", nullptr, &m_showHierarchyWindow);
            ImGui::MenuItem("Inspector", nullptr, &m_showInspectorWindow);
            ImGui::MenuItem("Animation", nullptr, &m_showAnimationWindow);
            ImGui::MenuItem("Content Browser", nullptr, &m_showContentBrowserWindow);
            ImGui::MenuItem("Procedural Texture", nullptr, &m_state.showProceduralTextureWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (m_showWorldWindow) {
        if (ImGui::Begin("World", &m_showWorldWindow)) {
        std::size_t entityCount = 0;
        const auto entityView = worldManager.GetActiveWorld().GetRegistry().view<OGLE::NameComponent>();
        for (auto entity : entityView) {
            (void)entity;
            ++entityCount;
        }

        const auto& position = camera.GetPosition();
        const auto& front = camera.GetFront();

        ImGui::Text("World entities: %u", static_cast<unsigned int>(entityCount));
        ImGui::Text("Physics bodies: %u", static_cast<unsigned int>(physicsManager.GetBodyCount()));
        ImGui::Text("Simulation: %s", m_simulationState == SimulationState::Playing ? "Playing" : "Paused");
        ImGui::Separator();
        ImGui::Text("Camera");
        ImGui::Text("Position: %.2f %.2f %.2f", position.x, position.y, position.z);
        ImGui::Text("Front: %.2f %.2f %.2f", front.x, front.y, front.z);
        ImGui::Separator();

        ImGui::Text("World Path");
        ImGui::InputText("##WorldPath", m_worldPathBuffer.data(), m_worldPathBuffer.size());

        if (ImGui::Button("Load World")) {
            configManager.GetConfig().world.path = m_worldPathBuffer.data();
            configManager.Save();
            worldManager.LoadActiveWorld(m_worldPathBuffer.data());
            m_selectedEntity = entt::null;
            m_bufferedEntity = entt::null;
            m_textureEditingEntity = entt::null;
        }
        ImGui::SameLine();
        if (ImGui::Button("Save World")) {
            configManager.GetConfig().world.path = m_worldPathBuffer.data();
            configManager.Save();
            worldManager.SaveActiveWorld(m_worldPathBuffer.data());
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload Default")) {
            worldManager.CreateDefaultWorld();
            m_selectedEntity = entt::null;
            m_bufferedEntity = entt::null;
            m_textureEditingEntity = entt::null;
        }

        if (ImGui::Button("Clear World")) {
            worldManager.ClearWorld();
            m_selectedEntity = entt::null;
            m_bufferedEntity = entt::null;
            m_textureEditingEntity = entt::null;
        }

        if (ImGui::Button(m_simulationState == SimulationState::Playing ? "Pause Simulation" : "Play Simulation")) {
            m_simulationState = (m_simulationState == SimulationState::Playing)
                ? SimulationState::Paused
                : SimulationState::Playing;
        }
        ImGui::SameLine();
        if (ImGui::Button("Step Simulation")) {
            m_simulationState = SimulationState::Paused;
            m_stepSimulationRequested = true;
        }

        ImGui::Separator();
        m_creationPanel.Draw(m_state, worldManager);

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GetContentBrowserAssetPayload())) {
                const char* assetPath = static_cast<const char*>(payload->Data);
                if (assetPath && IsEditorModelAssetPath(assetPath)) {
                    const std::string entityName = BuildEditorEntityNameFromAssetPath(assetPath);
                    const OGLE::Entity entity = worldManager.CreateModelFromFile(
                        assetPath,
                        OGLE::ModelType::DYNAMIC,
                        entityName);

                    if (entity != entt::null) {
                        const glm::vec3 spawnPosition = camera.GetPosition() + camera.GetFront() * 5.0f;
                        auto& world = worldManager.GetActiveWorld();
                        if (auto* transform = world.GetComponent<OGLE::TransformComponent>(entity)) {
                            world.SetTransform(entity, spawnPosition, transform->rotation, transform->scale);
                        }
                        m_selectedEntity = entity;
                        m_bufferedEntity = entt::null;
                        m_textureEditingEntity = entt::null;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        }
        ImGui::End();
    }

    if (m_showHierarchyWindow) {
        if (ImGui::Begin("Hierarchy", &m_showHierarchyWindow)) {
            m_hierarchyPanel.Draw(m_state, worldManager, physicsManager);
        }
        ImGui::End();
    }

    if (m_showInspectorWindow) {
        if (ImGui::Begin("Inspector", &m_showInspectorWindow)) {
            m_inspectorPanel.Draw(m_state, worldManager, physicsManager, cameraManager);
        }
        ImGui::End();
    }

    if (m_showAnimationWindow) {
        if (ImGui::Begin("Animation", &m_showAnimationWindow)) {
            m_animationPanel.Draw(m_state, worldManager);
        }
        ImGui::End();
    }

    if (m_showContentBrowserWindow) {
        if (ImGui::Begin("Content Browser", &m_showContentBrowserWindow)) {
            m_contentBrowserPanel.Draw(m_state, worldManager, configManager);
        }
        ImGui::End();
    }

    if (m_state.showProceduralTextureWindow) {
        if (ImGui::Begin("Procedural Texture Generator", &m_state.showProceduralTextureWindow)) {
            m_proceduralTexturePanel.Draw(m_state);
        }
        ImGui::End();
    }
}

void Editor::SyncSelectedBuffers(WorldManager& worldManager)
{
    if (m_selectedEntity == entt::null || !worldManager.IsEntityValid(m_selectedEntity)) {
        return;
    }

    if (m_bufferedEntity != m_selectedEntity) {
        m_bufferedEntity = m_selectedEntity;

        const auto selectedObject = worldManager.GetWorldObject(m_selectedEntity);
        m_selectedNameBuffer.fill('\0');
        std::strncpy(m_selectedNameBuffer.data(), selectedObject.GetName().c_str(), m_selectedNameBuffer.size() - 1);

        if (const OGLE::PrimitiveComponent* primitive = worldManager.GetActiveWorld().GetComponent<OGLE::PrimitiveComponent>(m_selectedEntity)) {
            m_primitiveSourcePathBuffer.fill('\0');
            std::strncpy(m_primitiveSourcePathBuffer.data(), primitive->sourcePath.c_str(), m_primitiveSourcePathBuffer.size() - 1);
        } else {
            m_primitiveSourcePathBuffer.fill('\0');
        }

        if (const OGLE::SkeletonComponent* skeleton = worldManager.GetActiveWorld().GetComponent<OGLE::SkeletonComponent>(m_selectedEntity)) {
            m_skeletonSourcePathBuffer.fill('\0');
            std::strncpy(m_skeletonSourcePathBuffer.data(), skeleton->sourcePath.c_str(), m_skeletonSourcePathBuffer.size() - 1);
        } else {
            m_skeletonSourcePathBuffer.fill('\0');
        }

        if (const OGLE::AnimationComponent* animation = worldManager.GetActiveWorld().GetComponent<OGLE::AnimationComponent>(m_selectedEntity)) {
            m_animationClipBuffer.fill('\0');
            std::strncpy(m_animationClipBuffer.data(), animation->currentClip.c_str(), m_animationClipBuffer.size() - 1);
        } else {
            m_animationClipBuffer.fill('\0');
        }

        if (const OGLE::ScriptComponent* script = worldManager.GetActiveWorld().GetComponent<OGLE::ScriptComponent>(m_selectedEntity)) {
            m_scriptPathBuffer.fill('\0');
            std::strncpy(m_scriptPathBuffer.data(), script->scriptPath.c_str(), m_scriptPathBuffer.size() - 1);
        } else {
            m_scriptPathBuffer.fill('\0');
        }

        if (OGLE::MaterialComponent* materialComponent = worldManager.GetActiveWorld().GetComponent<OGLE::MaterialComponent>(m_selectedEntity)) {
            m_textureEditingEntity = m_selectedEntity;
            const OGLE::Material& material = materialComponent->material;
            m_texturePathBuffer.fill('\0');
            m_emissiveTexturePathBuffer.fill('\0');
            std::strncpy(m_texturePathBuffer.data(), material.GetDiffuseTexturePath().c_str(), m_texturePathBuffer.size() - 1);
            std::strncpy(m_emissiveTexturePathBuffer.data(), material.GetEmissiveTexturePath().c_str(), m_emissiveTexturePathBuffer.size() - 1);
            m_baseColorBuffer = material.GetBaseColor();
            m_emissiveColorBuffer = material.GetEmissiveColor();
            m_uvTilingBuffer = material.GetUvTiling();
            m_uvOffsetBuffer = material.GetUvOffset();
            m_roughnessBuffer = material.GetRoughness();
            m_metallicBuffer = material.GetMetallic();
            m_alphaCutoffBuffer = material.GetAlphaCutoff();
            m_shaderProgramBuffer.fill('\0');
            std::strncpy(m_shaderProgramBuffer.data(), material.GetShaderProgram().c_str(), m_shaderProgramBuffer.size() - 1);
        } else if (OGLE::ModelEntity* model = selectedObject.GetModel()) {
            m_textureEditingEntity = m_selectedEntity;
            const OGLE::Material& material = model->GetMaterial();
            m_texturePathBuffer.fill('\0');
            m_emissiveTexturePathBuffer.fill('\0');
            std::strncpy(m_texturePathBuffer.data(), material.GetDiffuseTexturePath().c_str(), m_texturePathBuffer.size() - 1);
            std::strncpy(m_emissiveTexturePathBuffer.data(), material.GetEmissiveTexturePath().c_str(), m_emissiveTexturePathBuffer.size() - 1);
            m_baseColorBuffer = material.GetBaseColor();
            m_emissiveColorBuffer = material.GetEmissiveColor();
            m_uvTilingBuffer = material.GetUvTiling();
            m_uvOffsetBuffer = material.GetUvOffset();
            m_roughnessBuffer = material.GetRoughness();
            m_metallicBuffer = material.GetMetallic();
            m_alphaCutoffBuffer = material.GetAlphaCutoff();
            m_shaderProgramBuffer.fill('\0');
            std::strncpy(m_shaderProgramBuffer.data(), material.GetShaderProgram().c_str(), m_shaderProgramBuffer.size() - 1);
        } else {
            m_textureEditingEntity = entt::null;
            m_texturePathBuffer.fill('\0');
            m_emissiveTexturePathBuffer.fill('\0');
            m_shaderProgramBuffer.fill('\0');
        }
    }
}

bool Editor::TrySelectObject(const ogle::Camera& camera, WorldManager& worldManager)
{
    auto& input = ogle::InputController::Get();
    if (!input.IsMouseButtonPressed(static_cast<int>(ogle::MouseButton::Left))) {
        return false;
    }

    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    if (displaySize.x <= 1.0f || displaySize.y <= 1.0f) {
        return false;
    }

    const glm::vec2 mousePosition = input.GetMousePosition();
    const float x = (2.0f * mousePosition.x) / displaySize.x - 1.0f;
    const float y = 1.0f - (2.0f * mousePosition.y) / displaySize.y;

    const glm::mat4 inverseViewProjection = glm::inverse(camera.GetProjectionMatrix() * camera.GetViewMatrix());
    const glm::vec4 nearPoint = inverseViewProjection * glm::vec4(x, y, -1.0f, 1.0f);
    const glm::vec4 farPoint = inverseViewProjection * glm::vec4(x, y, 1.0f, 1.0f);

    if (nearPoint.w == 0.0f || farPoint.w == 0.0f) {
        return false;
    }

    const glm::vec3 rayOrigin = glm::vec3(nearPoint) / nearPoint.w;
    const glm::vec3 rayEnd = glm::vec3(farPoint) / farPoint.w;
    const glm::vec3 rayDirection = glm::normalize(rayEnd - rayOrigin);

    float closestHit = std::numeric_limits<float>::max();
    OGLE::Entity closestEntity = entt::null;

    worldManager.GetActiveWorld().GetRegistry().view<OGLE::NameComponent, OGLE::TransformComponent>().each(
        [&](auto entity, const OGLE::NameComponent&, const OGLE::TransformComponent& transform) {
            const glm::vec3 halfExtents = glm::max(transform.scale * 0.5f, glm::vec3(0.05f));
            const glm::vec3 boxMin = transform.position - halfExtents;
            const glm::vec3 boxMax = transform.position + halfExtents;

            float hitDistance = 0.0f;
            if (IntersectRayWithAabb(rayOrigin, rayDirection, boxMin, boxMax, hitDistance) && hitDistance < closestHit) {
                closestHit = hitDistance;
                closestEntity = entity;
            }
        });

    m_selectedEntity = closestEntity;
    return closestEntity != entt::null;
}

bool Editor::IntersectRayWithAabb(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    const glm::vec3& boxMin,
    const glm::vec3& boxMax,
    float& hitDistance)
{
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();

    for (int axis = 0; axis < 3; ++axis) {
        const float origin = rayOrigin[axis];
        const float direction = rayDirection[axis];

        if (std::abs(direction) < 1e-6f) {
            if (origin < boxMin[axis] || origin > boxMax[axis]) {
                return false;
            }
            continue;
        }

        float t1 = (boxMin[axis] - origin) / direction;
        float t2 = (boxMax[axis] - origin) / direction;

        if (t1 > t2) {
            std::swap(t1, t2);
        }

        tMin = (t1 > tMin) ? t1 : tMin;
        tMax = (t2 < tMax) ? t2 : tMax;

        if (tMin > tMax) {
            return false;
        }
    }

    hitDistance = tMin;
    return tMax >= 0.0f;
}
