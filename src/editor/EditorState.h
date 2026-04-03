#pragma once

#include "world/WorldComponents.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <array>

enum class EditorSimulationState {
    Playing,
    Paused
};

struct EditorState
{
    bool initialized = false;
    bool enabled = true;
    bool showWorldWindow = true;
    bool showHierarchyWindow = true;
    bool showInspectorWindow = true;
    bool showContentBrowserWindow = true;
    bool showAnimationWindow = true;
    bool showProceduralTextureWindow = false;
    EditorSimulationState simulationState = EditorSimulationState::Playing;
    bool stepSimulationRequested = false;
    OGLE::Entity selectedEntity = entt::null;
    OGLE::Entity bufferedEntity = entt::null;
    OGLE::Entity textureEditingEntity = entt::null;
    std::array<char, 512> texturePathBuffer{};
    std::array<char, 512> emissiveTexturePathBuffer{};
    std::array<char, 256> worldPathBuffer{};
    std::array<char, 256> assetsPathBuffer{};
    std::array<char, 256> selectedNameBuffer{};
    std::array<char, 256> createNameBuffer{};
    std::array<char, 512> createModelPathBuffer{};
    std::array<char, 512> createTexturePathBuffer{};
    std::array<char, 512> contentSelectionBuffer{};
    std::array<char, 512> primitiveSourcePathBuffer{};
    std::array<char, 512> skeletonSourcePathBuffer{};
    std::array<char, 512> shaderProgramBuffer{};
    std::array<char, 256> animationClipBuffer{};
    std::array<char, 512> scriptPathBuffer{};
    glm::vec3 baseColorBuffer{ 1.0f, 1.0f, 1.0f };
    glm::vec3 emissiveColorBuffer{ 0.0f, 0.0f, 0.0f };
    glm::vec2 uvTilingBuffer{ 1.0f, 1.0f };
    glm::vec2 uvOffsetBuffer{ 0.0f, 0.0f };
    float roughnessBuffer = 0.7f;
    float metallicBuffer = 0.0f;
    float alphaCutoffBuffer = 0.0f;
    int createKind = 1;
};
