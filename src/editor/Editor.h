#pragma once

#include "editor/EditorAnimationPanel.h"
#include "editor/EditorContentBrowserPanel.h"
#include "editor/EditorCreationPanel.h"
#include "editor/EditorHierarchyPanel.h"
#include "editor/EditorInspectorPanel.h"
#include "editor/EditorState.h"
#include "world/WorldComponents.h"

#include "config/ConfigManager.h"

namespace ogle {
    class Camera;
}

class CameraManager;
class WorldManager;
class PhysicsManager;

class Editor
{
public:
    using SimulationState = EditorSimulationState;

    Editor() = default;
    ~Editor() = default;

    bool Initialize();
    void Shutdown();

    void SetEnabled(bool enabled);
    void Toggle();
    bool IsEnabled() const;
    bool IsInitialized() const;
    EditorState& GetState() { return m_state; }
    const EditorState& GetState() const { return m_state; }
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

    EditorState m_state{};
    EditorHierarchyPanel m_hierarchyPanel{};
    EditorInspectorPanel m_inspectorPanel{};
    EditorAnimationPanel m_animationPanel{};
    EditorCreationPanel m_creationPanel{};
    EditorContentBrowserPanel m_contentBrowserPanel{};

    // Phase 1 of decomposition keeps old member names as aliases so behavior stays stable
    // while editor state now lives in a dedicated EditorState object.
    bool& m_initialized = m_state.initialized;
    bool& m_enabled = m_state.enabled;
    bool& m_showWorldWindow = m_state.showWorldWindow;
    bool& m_showHierarchyWindow = m_state.showHierarchyWindow;
    bool& m_showInspectorWindow = m_state.showInspectorWindow;
    bool& m_showAnimationWindow = m_state.showAnimationWindow;
    bool& m_showContentBrowserWindow = m_state.showContentBrowserWindow;
    SimulationState& m_simulationState = m_state.simulationState;
    bool& m_stepSimulationRequested = m_state.stepSimulationRequested;
    OGLE::Entity& m_selectedEntity = m_state.selectedEntity;
    OGLE::Entity& m_bufferedEntity = m_state.bufferedEntity;
    OGLE::Entity& m_textureEditingEntity = m_state.textureEditingEntity;
    std::array<char, 512>& m_texturePathBuffer = m_state.texturePathBuffer;
    std::array<char, 512>& m_emissiveTexturePathBuffer = m_state.emissiveTexturePathBuffer;
    std::array<char, 256>& m_worldPathBuffer = m_state.worldPathBuffer;
    std::array<char, 256>& m_assetsPathBuffer = m_state.assetsPathBuffer;
    std::array<char, 256>& m_selectedNameBuffer = m_state.selectedNameBuffer;
    std::array<char, 256>& m_createNameBuffer = m_state.createNameBuffer;
    std::array<char, 512>& m_createModelPathBuffer = m_state.createModelPathBuffer;
    std::array<char, 512>& m_createTexturePathBuffer = m_state.createTexturePathBuffer;
    std::array<char, 512>& m_contentSelectionBuffer = m_state.contentSelectionBuffer;
    std::array<char, 512>& m_primitiveSourcePathBuffer = m_state.primitiveSourcePathBuffer;
    std::array<char, 512>& m_skeletonSourcePathBuffer = m_state.skeletonSourcePathBuffer;
    std::array<char, 512>& m_shaderProgramBuffer = m_state.shaderProgramBuffer;
    std::array<char, 256>& m_animationClipBuffer = m_state.animationClipBuffer;
    std::array<char, 512>& m_scriptPathBuffer = m_state.scriptPathBuffer;
    glm::vec3& m_baseColorBuffer = m_state.baseColorBuffer;
    glm::vec3& m_emissiveColorBuffer = m_state.emissiveColorBuffer;
    glm::vec2& m_uvTilingBuffer = m_state.uvTilingBuffer;
    glm::vec2& m_uvOffsetBuffer = m_state.uvOffsetBuffer;
    float& m_roughnessBuffer = m_state.roughnessBuffer;
    float& m_metallicBuffer = m_state.metallicBuffer;
    float& m_alphaCutoffBuffer = m_state.alphaCutoffBuffer;
    int& m_createKind = m_state.createKind;
};
