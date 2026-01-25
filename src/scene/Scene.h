// src/scene/Scene.h
#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "SceneComponent.h"      // Базовый компонент
#include "Mesh.h"                // Mesh компонент
#include "Components.h"          // Struct-компоненты (Transform, Hierarchy, Tag, PhysicsBody)
#include "log/Logger.h"
#include "render/Camera.h"    // Для камеры и culling

namespace ogle {

class Scene {
public:
    entt::registry registry;

    Scene();
    ~Scene();

    void Initialize(Camera* mainCamera);  // Инит + установка камеры для culling
    entt::entity CreateEntity(const std::string& name = "");
    void DestroyEntity(entt::entity e);

    void Update(float deltaTime);
    void Render(float time, Camera* camera);  // Рендер с time и камерой (сцена берёт матрицы сама)

private:
    Camera* m_mainCamera = nullptr;  // Для frustum culling и матриц

    void UpdateHierarchy();
    void UpdatePhysics(float dt);
    // ... другие private системы, если нужно (анимация, скрипты)
};

} // namespace ogle