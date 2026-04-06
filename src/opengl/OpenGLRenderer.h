#pragma once
#ifndef OPENGLRENDERER_H_
#define OPENGLRENDERER_H_

#include "GLFunctions.h"
#include "ShaderManager.h"
#include "../world/WorldComponents.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <memory>
// Needed for std::chrono::steady_clock used in the implementation
#include <chrono>

namespace OGLE {
    class Camera;
}

namespace OGLE {
    class World;
    using Entity = entt::entity;
}

class WorldManager;

class OpenGLRenderer {
public:
    OpenGLRenderer(int width, int height, OGLE::Camera& camera, WorldManager& worldManager);
    ~OpenGLRenderer();

    bool Initialize();
    void Render();
    void Resize(int width, int height);
    void SetHighlightedEntity(OGLE::Entity entity);

private:
    struct LightingState {
        bool hasDirectionalLight = false;
        glm::vec3 directionalDirection{ -0.4f, -1.0f, -0.3f };
        glm::vec3 directionalColor{ 1.0f, 0.96f, 0.9f };
        float directionalIntensity = 1.5f;
        bool castsShadows = false;
        glm::mat4 lightSpaceMatrix{ 1.0f };
    };

    bool InitializeShadowResources();
    void DestroyShadowResources();
    void CollectLightingState(LightingState& lightingState);
    void RenderShadowPass(const LightingState& lightingState);
    glm::vec3 RotationToDirection(const glm::vec3& rotationDegrees) const;

    ShaderManager m_shaderManager;
    OGLE::Camera& m_camera;
    WorldManager& m_worldManager;
    int m_width;
    int m_height;
    OGLE::Entity m_highlightedEntity;
    GLuint m_shadowFramebuffer = 0;
    GLuint m_shadowDepthTexture = 0;
    int m_shadowMapSize = 2048;

    // std::unique_ptr<DomoScene> m_scene;
    // Time point marking when the renderer was created, used for delta time calculation
    std::chrono::steady_clock::time_point m_startTime;
};

#endif // OPENGLRENDERER_H_
