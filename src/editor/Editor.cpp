#include "editor/Editor.h"

#include "core/EventBus.h"
#include "core/Events.h"
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
    if (m_state.initialized) {
        return true;
    }

    m_state.worldPathBuffer.fill('\0');
    m_state.assetsPathBuffer.fill('\0');
    m_state.selectedNameBuffer.fill('\0');
    m_state.createNameBuffer.fill('\0');
    m_state.createModelPathBuffer.fill('\0');
    m_state.createTexturePathBuffer.fill('\0');
    m_state.contentSelectionBuffer.fill('\0');
    m_state.primitiveSourcePathBuffer.fill('\0');
    m_state.skeletonSourcePathBuffer.fill('\0');
    m_state.animationClipBuffer.fill('\0');
    m_state.scriptPathBuffer.fill('\0');
    strncpy(m_state.createNameBuffer.data(), "NewObject", m_state.createNameBuffer.size() - 1);
    strncpy(m_state.worldPathBuffer.data(), "assets/worlds/default_world.json", m_state.worldPathBuffer.size() - 1);
    strncpy(m_state.assetsPathBuffer.data(), "assets", m_state.assetsPathBuffer.size() - 1);

    m_state.initialized = true;
    LOG_INFO("Editor initialized");
    return true;
}

void Editor::Shutdown()
{
    if (!m_state.initialized) {
        return;
    }

    m_state.initialized = false;
    LOG_INFO("Editor shutdown");
}

void Editor::SetEnabled(bool enabled)
{
    m_state.enabled = enabled;
}

void Editor::Toggle()
{
    m_state.enabled = !m_state.enabled;
}

bool Editor::IsEnabled() const
{
    return m_state.enabled;
}

bool Editor::IsInitialized() const
{
    return m_state.initialized;
}

OGLE::Entity Editor::GetSelectedEntity() const
{
    return m_state.selectedEntity;
}

EditorSimulationState Editor::GetSimulationState() const
{
    return m_state.simulationState;
}

bool Editor::ConsumeSimulationStepRequest()
{
    const bool shouldStep = m_state.stepSimulationRequested;
    m_state.stepSimulationRequested = false;
    return shouldStep;
}

void Editor::BuildUi(
    const CameraManager& cameraManager,
    WorldManager& worldManager,
    PhysicsManager& physicsManager,
    ConfigManager& configManager)
{
    if (!m_state.initialized || !m_state.enabled) {
        return;
    }

    const OGLE::Camera& camera = cameraManager.GetCamera();
    if (!ImGui::GetIO().WantCaptureMouse) {
        TrySelectObject(camera, worldManager);
    }

    if (m_state.selectedEntity != entt::null && !worldManager.IsEntityValid(m_state.selectedEntity)) {
        m_state.selectedEntity = entt::null;
        m_state.bufferedEntity = entt::null;
        m_state.textureEditingEntity = entt::null;
    }

    if (m_state.worldPathBuffer[0] == '\0') {
        strncpy(
            m_state.worldPathBuffer.data(),
            configManager.GetConfig().world.path.c_str(),
            m_state.worldPathBuffer.size() - 1);
    }
    if (m_state.assetsPathBuffer[0] == '\0') {
        strncpy(
            m_state.assetsPathBuffer.data(),
            configManager.GetConfig().assets.path.c_str(),
            m_state.assetsPathBuffer.size() - 1);
    }

    SyncSelectedBuffers(worldManager);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load World")) {
                m_state.eventBus->Dispatch(OGLE::EditorLoadWorldEvent{ m_state.worldPathBuffer.data() });
            }
            if (ImGui::MenuItem("Save World")) {
                m_state.eventBus->Dispatch(OGLE::EditorSaveWorldEvent{ m_state.worldPathBuffer.data() });
            }
            if (ImGui::MenuItem("Reload Default")) {
                m_state.eventBus->Dispatch(OGLE::EditorReloadDefaultWorldEvent{});
            }
            if (ImGui::MenuItem("Clear World")) {
                m_state.eventBus->Dispatch(OGLE::EditorClearWorldEvent{});
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Simulation")) {
            const bool isPlaying = m_state.simulationState == EditorSimulationState::Playing;
            if (ImGui::MenuItem("Play", nullptr, isPlaying)) {
                m_state.eventBus->Dispatch(OGLE::EditorPlayEvent{});
            }
            if (ImGui::MenuItem("Pause", nullptr, !isPlaying)) {
                m_state.eventBus->Dispatch(OGLE::EditorPauseEvent{});
            }
            if (ImGui::MenuItem("Step")) {
                m_state.eventBus->Dispatch(OGLE::EditorStepEvent{});
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("World", nullptr, &m_state.showWorldWindow);
            ImGui::MenuItem("Hierarchy", nullptr, &m_state.showHierarchyWindow);
            ImGui::MenuItem("Inspector", nullptr, &m_state.showInspectorWindow);
            ImGui::MenuItem("Animation", nullptr, &m_state.showAnimationWindow);
            ImGui::MenuItem("Content Browser", nullptr, &m_state.showContentBrowserWindow);
            ImGui::MenuItem("Procedural Texture", nullptr, &m_state.showProceduralTextureWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (m_state.showWorldWindow) {
        if (ImGui::Begin("World", &m_state.showWorldWindow)) {
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
        ImGui::Text("Simulation: %s", m_state.simulationState == EditorSimulationState::Playing ? "Playing" : "Paused");
        ImGui::Separator();
        ImGui::Text("Camera");
        ImGui::Text("Position: %.2f %.2f %.2f", position.x, position.y, position.z);
        ImGui::Text("Front: %.2f %.2f %.2f", front.x, front.y, front.z);
        ImGui::Separator();

        ImGui::Text("World Path");
        ImGui::InputText("##WorldPath", m_state.worldPathBuffer.data(), m_state.worldPathBuffer.size());

        if (ImGui::Button("Load World")) {
            m_state.eventBus->Dispatch(OGLE::EditorLoadWorldEvent{ m_state.worldPathBuffer.data() });
        }
        ImGui::SameLine();
        if (ImGui::Button("Save World")) {
            m_state.eventBus->Dispatch(OGLE::EditorSaveWorldEvent{ m_state.worldPathBuffer.data() });
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload Default")) {
            m_state.eventBus->Dispatch(OGLE::EditorReloadDefaultWorldEvent{});
        }

        if (ImGui::Button("Clear World")) {
            m_state.eventBus->Dispatch(OGLE::EditorClearWorldEvent{});
        }

        if (ImGui::Button(m_state.simulationState == EditorSimulationState::Playing ? "Pause Simulation" : "Play Simulation")) {
            if (m_state.simulationState == EditorSimulationState::Playing) {
                m_state.eventBus->Dispatch(OGLE::EditorPauseEvent{});
            } else {
                m_state.eventBus->Dispatch(OGLE::EditorPlayEvent{});
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Step Simulation")) {
            m_state.eventBus->Dispatch(OGLE::EditorStepEvent{});
        }

        ImGui::Separator();
        m_creationPanel.Draw(m_state, worldManager);

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GetContentBrowserAssetPayload())) {
                const char* assetPath = static_cast<const char*>(payload->Data);
                if (assetPath && IsEditorModelAssetPath(assetPath)) {
                    m_state.eventBus->Dispatch(OGLE::EditorSpawnModelFromDragDropEvent{ std::string(assetPath) });
                }
            }
            ImGui::EndDragDropTarget();
        }
        }
        ImGui::End();
    }

    if (m_state.showHierarchyWindow) {
        if (ImGui::Begin("Hierarchy", &m_state.showHierarchyWindow)) {
            m_hierarchyPanel.Draw(m_state, worldManager, physicsManager);
        }
        ImGui::End();
    }

    if (m_state.showInspectorWindow) {
        if (ImGui::Begin("Inspector", &m_state.showInspectorWindow)) {
            m_inspectorPanel.Draw(m_state, worldManager, physicsManager, cameraManager);
        }
        ImGui::End();
    }

    if (m_state.showAnimationWindow) {
        if (ImGui::Begin("Animation", &m_state.showAnimationWindow)) {
            m_animationPanel.Draw(m_state, worldManager);
        }
        ImGui::End();
    }

    if (m_state.showContentBrowserWindow) {
        if (ImGui::Begin("Content Browser", &m_state.showContentBrowserWindow)) {
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
    if (m_state.selectedEntity == entt::null || !worldManager.IsEntityValid(m_state.selectedEntity)) {
        return;
    }

    if (m_state.bufferedEntity != m_state.selectedEntity) {
        m_state.bufferedEntity = m_state.selectedEntity;

        const auto selectedObject = worldManager.GetWorldObject(m_state.selectedEntity);
        m_state.selectedNameBuffer.fill('\0');
        strncpy(m_state.selectedNameBuffer.data(), selectedObject.GetName().c_str(), m_state.selectedNameBuffer.size() - 1);

        if (const OGLE::PrimitiveComponent* primitive = worldManager.GetActiveWorld().GetComponent<OGLE::PrimitiveComponent>(m_state.selectedEntity)) {
            m_state.primitiveSourcePathBuffer.fill('\0');
            strncpy(m_state.primitiveSourcePathBuffer.data(), primitive->sourcePath.c_str(), m_state.primitiveSourcePathBuffer.size() - 1);
        } else {
            m_state.primitiveSourcePathBuffer.fill('\0');
        }

        if (const OGLE::SkeletonComponent* skeleton = worldManager.GetActiveWorld().GetComponent<OGLE::SkeletonComponent>(m_state.selectedEntity)) {
            m_state.skeletonSourcePathBuffer.fill('\0');
            strncpy(m_state.skeletonSourcePathBuffer.data(), skeleton->sourcePath.c_str(), m_state.skeletonSourcePathBuffer.size() - 1);
        } else {
            m_state.skeletonSourcePathBuffer.fill('\0');
        }

        if (const OGLE::AnimationComponent* animation = worldManager.GetActiveWorld().GetComponent<OGLE::AnimationComponent>(m_state.selectedEntity)) {
            m_state.animationClipBuffer.fill('\0');
            strncpy(m_state.animationClipBuffer.data(), animation->currentClip.c_str(), m_state.animationClipBuffer.size() - 1);
        } else {
            m_state.animationClipBuffer.fill('\0');
        }

        if (const OGLE::ScriptComponent* script = worldManager.GetActiveWorld().GetComponent<OGLE::ScriptComponent>(m_state.selectedEntity)) {
            m_state.scriptPathBuffer.fill('\0');
            strncpy(m_state.scriptPathBuffer.data(), script->scriptPath.c_str(), m_state.scriptPathBuffer.size() - 1);
        } else {
            m_state.scriptPathBuffer.fill('\0');
        }

        if (OGLE::MaterialComponent* materialComponent = worldManager.GetActiveWorld().GetComponent<OGLE::MaterialComponent>(m_state.selectedEntity)) {
            m_state.textureEditingEntity = m_state.selectedEntity;
            const OGLE::Material& material = materialComponent->material;

            m_state.materialTexturePathsBuffer.clear();
            const auto& texturePaths = material.GetTexturePaths();
            for (const auto& pair : texturePaths) {
                m_state.materialTexturePathsBuffer[pair.first].fill('\0');
                strncpy(m_state.materialTexturePathsBuffer[pair.first].data(), pair.second.c_str(), m_state.materialTexturePathsBuffer[pair.first].size() - 1);
            }

            m_state.baseColorBuffer = material.GetBaseColor();
            m_state.emissiveColorBuffer = material.GetEmissiveColor();
            m_state.uvTilingBuffer = material.GetUvTiling();
            m_state.uvOffsetBuffer = material.GetUvOffset();
            m_state.roughnessBuffer = material.GetRoughness();
            m_state.metallicBuffer = material.GetMetallic();
            m_state.alphaCutoffBuffer = material.GetAlphaCutoff();
            m_state.shaderProgramBuffer.fill('\0');
            strncpy(m_state.shaderProgramBuffer.data(), material.GetShaderProgram().c_str(), m_state.shaderProgramBuffer.size() - 1);
        } else if (OGLE::ModelEntity* model = selectedObject.GetModel()) {
            m_state.textureEditingEntity = m_state.selectedEntity;
            const OGLE::Material& material = model->GetMaterial();

            m_state.materialTexturePathsBuffer.clear();
            const auto& texturePaths = material.GetTexturePaths();
            for (const auto& pair : texturePaths) {
                m_state.materialTexturePathsBuffer[pair.first].fill('\0');
                strncpy(m_state.materialTexturePathsBuffer[pair.first].data(), pair.second.c_str(), m_state.materialTexturePathsBuffer[pair.first].size() - 1);
            }

            m_state.baseColorBuffer = material.GetBaseColor();
            m_state.emissiveColorBuffer = material.GetEmissiveColor();
            m_state.uvTilingBuffer = material.GetUvTiling();
            m_state.uvOffsetBuffer = material.GetUvOffset();
            m_state.roughnessBuffer = material.GetRoughness();
            m_state.metallicBuffer = material.GetMetallic();
            m_state.alphaCutoffBuffer = material.GetAlphaCutoff();
            m_state.shaderProgramBuffer.fill('\0');
            strncpy(m_state.shaderProgramBuffer.data(), material.GetShaderProgram().c_str(), m_state.shaderProgramBuffer.size() - 1);
        } else {
            m_state.textureEditingEntity = entt::null;
            m_state.materialTexturePathsBuffer.clear();
            m_state.shaderProgramBuffer.fill('\0');
        }
    }
}

bool Editor::TrySelectObject(const OGLE::Camera& camera, WorldManager& worldManager)
{
    auto& input = OGLE::InputController::Get();
    if (!input.IsMouseButtonPressed(static_cast<int>(OGLE::MouseButton::Left))) {
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

    m_state.selectedEntity = closestEntity;
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

void Editor::SubscribeToEvents(EventBus& eventBus,
    ConfigManager& configManager,
    WorldManager& worldManager,
    PhysicsManager& physicsManager,
    CameraManager& cameraManager)
{
    eventBus.Subscribe<OGLE::EditorLoadWorldEvent>([this, &configManager, &worldManager](const OGLE::EditorLoadWorldEvent& e) {
        configManager.GetConfig().world.path = e.path;
        configManager.Save();
        worldManager.LoadActiveWorld(e.path);
        GetState().selectedEntity = entt::null;
        GetState().bufferedEntity = entt::null;
        GetState().textureEditingEntity = entt::null;
    });

    eventBus.Subscribe<OGLE::EditorSaveWorldEvent>([this, &configManager, &worldManager](const OGLE::EditorSaveWorldEvent& e) {
        configManager.GetConfig().world.path = e.path;
        configManager.Save();
        worldManager.SaveActiveWorld(e.path);
    });

    eventBus.Subscribe<OGLE::EditorReloadDefaultWorldEvent>([this, &worldManager](const OGLE::EditorReloadDefaultWorldEvent&) {
        worldManager.CreateDefaultWorld();
        GetState().selectedEntity = entt::null;
        GetState().bufferedEntity = entt::null;
        GetState().textureEditingEntity = entt::null;
    });

    eventBus.Subscribe<OGLE::EditorClearWorldEvent>([this, &worldManager](const OGLE::EditorClearWorldEvent&) {
        worldManager.ClearWorld();
        GetState().selectedEntity = entt::null;
        GetState().bufferedEntity = entt::null;
        GetState().textureEditingEntity = entt::null;
    });

    eventBus.Subscribe<OGLE::EditorPlayEvent>([this](const OGLE::EditorPlayEvent&) {
        GetState().simulationState = Editor::SimulationState::Playing;
    });

    eventBus.Subscribe<OGLE::EditorPauseEvent>([this](const OGLE::EditorPauseEvent&) {
        GetState().simulationState = Editor::SimulationState::Paused;
    });

    eventBus.Subscribe<OGLE::EditorStepEvent>([this](const OGLE::EditorStepEvent&) {
        GetState().simulationState = Editor::SimulationState::Paused;
        GetState().stepSimulationRequested = true;
    });

    eventBus.Subscribe<OGLE::EditorCreateEntityEvent>([this, &worldManager](const OGLE::EditorCreateEntityEvent& e) {
        entt::entity created = entt::null;

        switch (e.type) {
            case OGLE::EditorCreateEntityEvent::Type::EmptyObject:
                created = worldManager.CreateWorldObject(e.name, OGLE::WorldObjectKind::Generic).GetEntity();
                break;
            case OGLE::EditorCreateEntityEvent::Type::Cube:
                created = worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Cube,
                    glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), e.texturePath);
                break;
            case OGLE::EditorCreateEntityEvent::Type::Sphere:
                created = worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Sphere,
                    glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), e.texturePath);
                break;
            case OGLE::EditorCreateEntityEvent::Type::Plane:
                created = worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Plane,
                    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(5.0f, 1.0f, 5.0f), e.texturePath);
                break;
            case OGLE::EditorCreateEntityEvent::Type::DirectionalLight:
                created = worldManager.CreateDirectionalLight(e.name, glm::vec3(-50.0f, 45.0f, 0.0f));
                break;
            case OGLE::EditorCreateEntityEvent::Type::PointLight:
                created = worldManager.CreatePointLight(e.name, glm::vec3(0.0f, 1.5f, 0.0f));
                break;
            case OGLE::EditorCreateEntityEvent::Type::ModelFromFile:
                created = worldManager.CreateModelFromFile(e.modelPath, OGLE::ModelType::DYNAMIC, e.name);
                if (created != entt::null && !e.texturePath.empty()) {
                    // worldManager.SetEntityDiffuseTexture(created, e.texturePath);
                }
                break;
        }

        if (created != entt::null) {
            GetState().selectedEntity = created;
            GetState().bufferedEntity = entt::null;
            GetState().textureEditingEntity = entt::null;
        }
    });

    eventBus.Subscribe<OGLE::EditorDeleteEntityEvent>([this, &worldManager, &physicsManager](const OGLE::EditorDeleteEntityEvent& e) {
        if (e.entity != entt::null && worldManager.IsEntityValid(e.entity)) {
            physicsManager.RemoveBody(e.entity);
            worldManager.GetActiveWorld().DestroyEntity(e.entity);
        }
        if (e.entity == GetState().selectedEntity) {
            GetState().selectedEntity = entt::null;
            GetState().bufferedEntity = entt::null;
            GetState().textureEditingEntity = entt::null;
        }
    });

    eventBus.Subscribe<OGLE::EditorSpawnModelFromDragDropEvent>([this, &worldManager, &cameraManager](const OGLE::EditorSpawnModelFromDragDropEvent& e) {
        // Reuse camera + spawn logic from the original drag-drop handler
        const auto& camera = cameraManager.GetCamera();
        const glm::vec3 spawnPosition = camera.GetPosition() + camera.GetFront() * 5.0f;
        const OGLE::Entity entity = worldManager.CreateModelFromFile(
            e.assetPath, OGLE::ModelType::DYNAMIC, BuildEditorEntityNameFromAssetPath(e.assetPath));

        if (entity != entt::null) {
            auto& world = worldManager.GetActiveWorld();
            if (auto* transform = world.GetComponent<OGLE::TransformComponent>(entity)) {
                world.SetTransform(entity, spawnPosition, transform->rotation, transform->scale);
            }
            GetState().selectedEntity = entity;
            GetState().bufferedEntity = entt::null;
            GetState().textureEditingEntity = entt::null;
        }
    });

    eventBus.Subscribe<OGLE::EditorTransformChangedEvent>([this, &worldManager](const OGLE::EditorTransformChangedEvent& e) {
        if (e.entity != entt::null && worldManager.IsEntityValid(e.entity)) {
            worldManager.GetWorldObject(e.entity).SetTransform(e.position, e.rotation, e.scale);
        }
    });

    eventBus.Subscribe<OGLE::EditorNameChangedEvent>([this, &worldManager](const OGLE::EditorNameChangedEvent& e) {
        if (e.entity != entt::null && worldManager.IsEntityValid(e.entity)) {
            worldManager.GetWorldObject(e.entity).SetName(e.name);
        }
    });
}
