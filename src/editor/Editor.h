#pragma once

#include "world/WorldComponents.h"

#include "config/ConfigManager.h"

#include <glm/vec3.hpp>
#include <array>
#include <filesystem>

namespace ogle {
    class Camera;
}

class CameraManager;
class WorldManager;
class PhysicsManager;

class Editor
{
public:
    enum class SimulationState {
        Playing,
        Paused
    };

    Editor() = default;
    ~Editor() = default;

    bool Initialize();
    void Shutdown();

    void SetEnabled(bool enabled);
    void Toggle();
    bool IsEnabled() const;
    bool IsInitialized() const;
    OGLE::Entity GetSelectedEntity() const;
    SimulationState GetSimulationState() const;
    bool ConsumeSimulationStepRequest();

    void BuildUi(
        const CameraManager& cameraManager,
        WorldManager& worldManager,
        PhysicsManager& physicsManager,
        ConfigManager& configManager);

private:
    bool TrySelectObject(const ogle::Camera& camera, WorldManager& worldManager);
    static bool IntersectRayWithAabb(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        const glm::vec3& boxMin,
        const glm::vec3& boxMax,
        float& hitDistance);
    void SyncSelectedBuffers(WorldManager& worldManager);
    static const char* GetKindLabel(OGLE::WorldObjectKind kind);
    void DrawWorldTree(WorldManager& worldManager, PhysicsManager& physicsManager);
    void DrawSelectionInspector(WorldManager& worldManager, PhysicsManager& physicsManager);
    void DrawCreationTools(WorldManager& worldManager);
    void DrawContentBrowser(ConfigManager& configManager);
    void DrawContentBrowserDirectory(const std::filesystem::path& directoryPath, const std::filesystem::path& rootPath);
    void HandleContentBrowserFileSelected(const std::filesystem::path& filePath, const std::filesystem::path& rootPath);
    static bool IsModelAssetPath(const std::string& path);
    static bool IsTextureAssetPath(const std::string& path);
    static std::string BuildEntityNameFromAssetPath(const std::string& path);

    bool m_initialized = false;
    bool m_enabled = true;
    bool m_showWorldWindow = true;
    bool m_showHierarchyWindow = true;
    bool m_showInspectorWindow = true;
    bool m_showContentBrowserWindow = true;
    SimulationState m_simulationState = SimulationState::Playing;
    bool m_stepSimulationRequested = false;
    OGLE::Entity m_selectedEntity = entt::null;
    OGLE::Entity m_bufferedEntity = entt::null;
    OGLE::Entity m_textureEditingEntity = entt::null;
    std::array<char, 512> m_texturePathBuffer{};
    std::array<char, 256> m_worldPathBuffer{};
    std::array<char, 256> m_assetsPathBuffer{};
    std::array<char, 256> m_selectedNameBuffer{};
    std::array<char, 256> m_createNameBuffer{};
    std::array<char, 512> m_createModelPathBuffer{};
    std::array<char, 512> m_createTexturePathBuffer{};
    std::array<char, 512> m_contentSelectionBuffer{};
    int m_createKind = 1;
};
