#include "editor/Editor.h"

#include "Logger.h"
#include "input/InputController.h"
#include "opengl/Camera.h"
#include "managers/CameraManager.h"
#include "managers/PhysicsManager.h"
#include "managers/WorldManager.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vector_relational.hpp>
#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <limits>
#include <string>
#include <utility>
#include <vector>

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
    std::strncpy(m_createNameBuffer.data(), "NewObject", m_createNameBuffer.size() - 1);
    std::strncpy(m_worldPathBuffer.data(), "data/worlds/default_world.json", m_worldPathBuffer.size() - 1);
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

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("World", nullptr, &m_showWorldWindow);
            ImGui::MenuItem("Hierarchy", nullptr, &m_showHierarchyWindow);
            ImGui::MenuItem("Inspector", nullptr, &m_showInspectorWindow);
            ImGui::MenuItem("Content Browser", nullptr, &m_showContentBrowserWindow);
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

        ImGui::Separator();
        DrawCreationTools(worldManager);
        }
        ImGui::End();
    }

    if (m_showHierarchyWindow) {
        if (ImGui::Begin("Hierarchy", &m_showHierarchyWindow)) {
            DrawWorldTree(worldManager);
        }
        ImGui::End();
    }

    if (m_showInspectorWindow) {
        if (ImGui::Begin("Inspector", &m_showInspectorWindow)) {
            DrawSelectionInspector(worldManager, physicsManager);
        }
        ImGui::End();
    }

    if (m_showContentBrowserWindow) {
        if (ImGui::Begin("Content Browser", &m_showContentBrowserWindow)) {
            DrawContentBrowser(configManager);
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

        if (OGLE::ModelEntity* model = selectedObject.GetModel()) {
            m_textureEditingEntity = m_selectedEntity;
            m_texturePathBuffer.fill('\0');
            std::strncpy(m_texturePathBuffer.data(), model->GetDiffuseTexturePath().c_str(), m_texturePathBuffer.size() - 1);
        } else {
            m_textureEditingEntity = entt::null;
            m_texturePathBuffer.fill('\0');
        }
    }
}

const char* Editor::GetKindLabel(OGLE::WorldObjectKind kind)
{
    switch (kind) {
        case OGLE::WorldObjectKind::Generic: return "Generic";
        case OGLE::WorldObjectKind::Mesh: return "Mesh";
        case OGLE::WorldObjectKind::Light: return "Light";
        case OGLE::WorldObjectKind::Billboard: return "Billboard";
        default: return "Unknown";
    }
}

void Editor::DrawWorldTree(WorldManager& worldManager)
{
    struct KindGroup {
        OGLE::WorldObjectKind kind;
        const char* label;
    };

    static const KindGroup groups[] = {
        { OGLE::WorldObjectKind::Generic, "Generic" },
        { OGLE::WorldObjectKind::Mesh, "Mesh" },
        { OGLE::WorldObjectKind::Light, "Light" },
        { OGLE::WorldObjectKind::Billboard, "Billboard" }
    };

    auto nameView = worldManager.GetActiveWorld().GetRegistry().view<OGLE::NameComponent, OGLE::WorldObjectComponent>();
    for (const KindGroup& group : groups) {
        std::vector<OGLE::Entity> entities;
        for (auto entity : nameView) {
            const auto& object = nameView.get<OGLE::WorldObjectComponent>(entity);
            if (object.kind == group.kind) {
                entities.push_back(entity);
            }
        }

        std::sort(entities.begin(), entities.end(), [&nameView](OGLE::Entity a, OGLE::Entity b) {
            return nameView.get<OGLE::NameComponent>(a).value < nameView.get<OGLE::NameComponent>(b).value;
        });

        const bool opened = ImGui::TreeNodeEx(group.label, 0);
        if (opened) {
            if (entities.empty()) {
                ImGui::TextDisabled("Empty");
            } else {
                for (OGLE::Entity entity : entities) {
                    const auto& name = nameView.get<OGLE::NameComponent>(entity);
                    const bool selected = entity == m_selectedEntity;
                    ImGuiTreeNodeFlags itemFlags =
                        ImGuiTreeNodeFlags_Leaf |
                        ImGuiTreeNodeFlags_NoTreePushOnOpen |
                        (selected ? ImGuiTreeNodeFlags_Selected : 0);

                    ImGui::TreeNodeEx(
                        reinterpret_cast<void*>(static_cast<intptr_t>(entt::to_integral(entity))),
                        itemFlags,
                        "%s##%u",
                        name.value.c_str(),
                        static_cast<unsigned int>(entt::to_integral(entity)));

                    if (ImGui::IsItemClicked()) {
                        m_selectedEntity = entity;
                    }
                }
            }

            ImGui::TreePop();
        }
    }
}

void Editor::DrawSelectionInspector(WorldManager& worldManager, PhysicsManager& physicsManager)
{
    if (m_selectedEntity == entt::null || !worldManager.IsEntityValid(m_selectedEntity)) {
        ImGui::TextUnformatted("No object selected.");
        return;
    }

    auto selectedObject = worldManager.GetWorldObject(m_selectedEntity);
    OGLE::World& world = worldManager.GetActiveWorld();
    OGLE::WorldObjectComponent* worldObject = world.GetWorldObjectComponent(m_selectedEntity);
    OGLE::TransformComponent currentTransform = selectedObject.GetTransform();
    glm::vec3 position = currentTransform.position;
    glm::vec3 rotation = currentTransform.rotation;
    glm::vec3 scale = currentTransform.scale;

    ImGui::Text("Entity: %u", static_cast<unsigned int>(entt::to_integral(m_selectedEntity)));
    ImGui::InputText("Name", m_selectedNameBuffer.data(), m_selectedNameBuffer.size());
    if (ImGui::Button("Apply Name")) {
        selectedObject.SetName(m_selectedNameBuffer.data());
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

    if (OGLE::ModelEntity* model = selectedObject.GetModel()) {
        ImGui::Separator();
        ImGui::Text("Model");
        ImGui::TextWrapped("Source: %s", model->GetFilePath().empty() ? "<procedural>" : model->GetFilePath().c_str());

        if (m_textureEditingEntity != m_selectedEntity) {
            m_textureEditingEntity = m_selectedEntity;
            m_texturePathBuffer.fill('\0');
            std::strncpy(m_texturePathBuffer.data(), model->GetDiffuseTexturePath().c_str(), m_texturePathBuffer.size() - 1);
        }

        ImGui::InputText("Diffuse Texture", m_texturePathBuffer.data(), m_texturePathBuffer.size());
        if (ImGui::Button("Apply Texture")) {
            worldManager.SetEntityDiffuseTexture(m_selectedEntity, m_texturePathBuffer.data());
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Texture")) {
            worldManager.SetEntityDiffuseTexture(m_selectedEntity, "");
            m_texturePathBuffer.fill('\0');
        }
    }

    OGLE::PhysicsBodyComponent* physicsBody = world.GetPhysicsBody(m_selectedEntity);
    if (physicsBody) {
        ImGui::Separator();
        ImGui::Text("Physics");

        int bodyType = static_cast<int>(physicsBody->type);
        float mass = physicsBody->mass;
        glm::vec3 halfExtents = physicsBody->halfExtents;
        bool simulate = physicsBody->simulate;

        const char* bodyTypeLabels[] = { "Static", "Dynamic", "Kinematic" };
        ImGui::Combo("Body Type", &bodyType, bodyTypeLabels, IM_ARRAYSIZE(bodyTypeLabels));
        ImGui::DragFloat("Mass", &mass, 0.1f, 0.0f, 1000.0f);
        ImGui::DragFloat3("Half Extents", glm::value_ptr(halfExtents), 0.05f, 0.01f, 1000.0f);
        ImGui::Checkbox("Simulate", &simulate);

        if (ImGui::Button("Apply Physics")) {
            physicsBody->type = static_cast<OGLE::PhysicsBodyType>(bodyType);
            physicsBody->mass = mass;
            physicsBody->halfExtents = halfExtents;
            physicsBody->simulate = simulate;

            if (simulate) {
                physicsManager.AddBoxBody(
                    m_selectedEntity,
                    halfExtents,
                    physicsBody->type,
                    mass);
            } else {
                physicsManager.RemoveBody(m_selectedEntity);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Physics")) {
            physicsManager.RemoveBody(m_selectedEntity);
            world.GetRegistry().remove<OGLE::PhysicsBodyComponent>(m_selectedEntity);
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Delete Selected")) {
        physicsManager.RemoveBody(m_selectedEntity);
        world.DestroyEntity(m_selectedEntity);
        m_selectedEntity = entt::null;
        m_bufferedEntity = entt::null;
        m_textureEditingEntity = entt::null;
    }
}

void Editor::DrawCreationTools(WorldManager& worldManager)
{
    ImGui::TextUnformatted("Create");
    ImGui::Separator();

    const char* createKinds[] = { "Empty Object", "Cube", "Model From File" };
    ImGui::Combo("Create Type", &m_createKind, createKinds, IM_ARRAYSIZE(createKinds));
    ImGui::InputText("New Name", m_createNameBuffer.data(), m_createNameBuffer.size());

    if (m_createKind == 2) {
        ImGui::InputText("Model Path", m_createModelPathBuffer.data(), m_createModelPathBuffer.size());
    }

    if (m_createKind == 1 || m_createKind == 2) {
        ImGui::InputText("Texture Path", m_createTexturePathBuffer.data(), m_createTexturePathBuffer.size());
    }

    if (ImGui::Button("Create")) {
        const std::string name = m_createNameBuffer[0] != '\0' ? m_createNameBuffer.data() : "NewObject";

        if (m_createKind == 0) {
            m_selectedEntity = worldManager.CreateWorldObject(name, OGLE::WorldObjectKind::Generic).GetEntity();
        } else if (m_createKind == 1) {
            m_selectedEntity = worldManager.CreateCube(
                name,
                glm::vec3(0.0f, 0.5f, 0.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                m_createTexturePathBuffer.data());
        } else {
            m_selectedEntity = worldManager.CreateModelFromFile(
                m_createModelPathBuffer.data(),
                OGLE::ModelType::DYNAMIC,
                name);

            if (m_selectedEntity != entt::null && m_createTexturePathBuffer[0] != '\0') {
                worldManager.SetEntityDiffuseTexture(m_selectedEntity, m_createTexturePathBuffer.data());
            }
        }

        m_bufferedEntity = entt::null;
        m_textureEditingEntity = entt::null;
    }
}

void Editor::DrawContentBrowser(ConfigManager& configManager)
{
    ImGui::InputText("Assets Root", m_assetsPathBuffer.data(), m_assetsPathBuffer.size());

    if (ImGui::Button("Save Assets Root")) {
        configManager.GetConfig().assets.path = m_assetsPathBuffer.data();
        configManager.Save();
    }

    const std::filesystem::path rootPath = std::filesystem::path(m_assetsPathBuffer.data());

    if (m_contentSelectionBuffer[0] != '\0') {
        ImGui::TextWrapped("Selected asset: %s", m_contentSelectionBuffer.data());
    }

    if (!std::filesystem::exists(rootPath)) {
        ImGui::TextDisabled("Folder not found: %s", rootPath.string().c_str());
        return;
    }

    ImGui::BeginChild("ContentBrowserTree", ImVec2(0.0f, 220.0f), true);
    DrawContentBrowserDirectory(rootPath, rootPath);
    ImGui::EndChild();
}

void Editor::DrawContentBrowserDirectory(const std::filesystem::path& directoryPath, const std::filesystem::path& rootPath)
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
                DrawContentBrowserDirectory(entryPath, rootPath);
                ImGui::TreePop();
            }
        } else {
            const std::string relativePath = std::filesystem::relative(entryPath, rootPath).generic_string();
            const bool selected = std::strcmp(m_contentSelectionBuffer.data(), relativePath.c_str()) == 0;
            if (ImGui::Selectable((label + "##" + relativePath).c_str(), selected)) {
                HandleContentBrowserFileSelected(entryPath, rootPath);
            }
        }
    }
}

void Editor::HandleContentBrowserFileSelected(const std::filesystem::path& filePath, const std::filesystem::path& rootPath)
{
    std::filesystem::path pathForUse = filePath;
    std::error_code errorCode;
    const std::filesystem::path relativeToCwd = std::filesystem::relative(filePath, std::filesystem::current_path(), errorCode);
    const std::string relativeToCwdString = relativeToCwd.generic_string();
    if (!errorCode && !relativeToCwd.empty() && relativeToCwdString.rfind("../", 0) != 0 && relativeToCwdString != "..") {
        pathForUse = relativeToCwd;
    }

    const std::string relativePath = std::filesystem::relative(filePath, rootPath).generic_string();
    const std::string usePathString = pathForUse.generic_string();

    m_contentSelectionBuffer.fill('\0');
    std::strncpy(m_contentSelectionBuffer.data(), usePathString.c_str(), m_contentSelectionBuffer.size() - 1);

    const std::string extension = filePath.extension().string();
    std::string loweredExtension = extension;
    std::transform(loweredExtension.begin(), loweredExtension.end(), loweredExtension.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (loweredExtension == ".png" || loweredExtension == ".jpg" || loweredExtension == ".jpeg" || loweredExtension == ".bmp" || loweredExtension == ".tga") {
        m_createTexturePathBuffer.fill('\0');
        m_texturePathBuffer.fill('\0');
        std::strncpy(m_createTexturePathBuffer.data(), usePathString.c_str(), m_createTexturePathBuffer.size() - 1);
        std::strncpy(m_texturePathBuffer.data(), usePathString.c_str(), m_texturePathBuffer.size() - 1);
    }

    if (loweredExtension == ".obj" || loweredExtension == ".fbx" || loweredExtension == ".glb" || loweredExtension == ".gltf" || loweredExtension == ".stl") {
        m_createModelPathBuffer.fill('\0');
        std::strncpy(m_createModelPathBuffer.data(), usePathString.c_str(), m_createModelPathBuffer.size() - 1);
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

    auto view = worldManager.GetActiveWorld().GetRegistry().view<OGLE::NameComponent, OGLE::TransformComponent>();
    for (auto entity : view) {
        const auto& transform = view.get<OGLE::TransformComponent>(entity);
        const glm::vec3 halfExtents = glm::max(transform.scale * 0.5f, glm::vec3(0.05f));
        const glm::vec3 boxMin = transform.position - halfExtents;
        const glm::vec3 boxMax = transform.position + halfExtents;

        float hitDistance = 0.0f;
        if (IntersectRayWithAabb(rayOrigin, rayDirection, boxMin, boxMax, hitDistance) && hitDistance < closestHit) {
            closestHit = hitDistance;
            closestEntity = entity;
        }
    }

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
