// src/scene/Scene.h
#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "SceneComponent.h"      // Базовый компонент
#include "Mesh.h"                // Mesh компонент
#include "Components.h"          // Struct-компоненты
#include "log/Logger.h"
#include "render/Camera.h"    // Для IsInFrustum

namespace ogle {

    class Scene {
    public:
        entt::registry registry;

        Scene();
        ~Scene();

        void Initialize(Camera* mainCamera);
        entt::entity CreateEntity(const std::string& name = "");
        void DestroyEntity(entt::entity e);

        void SetParent(entt::entity child, entt::entity parent);

        void Update(float deltaTime);
        void Render(float time, Camera* camera);

    private:
        Camera* m_mainCamera = nullptr;

        void UpdateHierarchy();
        void UpdateTransformHierarchy(entt::entity e);

        void RenderSubtree(entt::entity e, float time, const glm::mat4& view, const glm::mat4& proj, Camera* cam);

        void UpdatePhysics(float dt);
    };

} // namespace ogle