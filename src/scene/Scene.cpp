// src/scene/Scene.cpp
#include "Scene.h"

namespace ogle {

Scene::Scene() {
    Logger::Info("Scene created");
}

Scene::~Scene() {
    registry.clear();
    Logger::Info("Scene destroyed");
}

void Scene::Initialize(Camera* mainCamera) {
    m_mainCamera = mainCamera;
    
    // Пример: Создаём тестовый entity с Mesh
    auto testEntity = CreateEntity("TestMesh");
    auto& mesh = registry.emplace<Mesh>(testEntity);
    mesh.Initialize();
    mesh.CreateCube();  // Тестовый куб, если нужно; можно убрать или сделать опциональным
}

entt::entity Scene::CreateEntity(const std::string& name) {
    auto e = registry.create();
    registry.emplace<Transform>(e);
    if (!name.empty()) {
        registry.emplace<Tag>(e, name);
    }
    return e;
}

void Scene::DestroyEntity(entt::entity e) {
    if (registry.all_of<Hierarchy>(e)) {
        auto& hier = registry.get<Hierarchy>(e);
        for (auto child : hier.children) {
            DestroyEntity(child);
        }
    }
    registry.destroy(e);
}

void Scene::Update(float deltaTime) {
    UpdateHierarchy();
    UpdatePhysics(deltaTime);
    
    // Обновляем все SceneComponent (вызываем их Update)
    auto compView = registry.view<SceneComponent>();
    for (auto e : compView) {
        auto& comp = compView.get<SceneComponent>(e);
        comp.Update(deltaTime);
    }
}

void Scene::Render(float time, Camera* camera) {
    if (!camera) return;

    auto viewMatrix = camera->GetViewMatrix();
    auto projectionMatrix = camera->GetProjectionMatrix();

    // Перебираем все entity с Mesh и Transform
    auto meshView = registry.view<Mesh, Transform>();
    for (auto e : meshView) {
        auto& mesh = meshView.get<Mesh>(e);
        auto& tr = meshView.get<Transform>(e);
        
        if (!mesh.visible) continue;
        
        // Frustum culling: Проверяем видимость в камере
        // if (!camera->IsInFrustum(tr.position, mesh.GetBoundingRadius())) continue;
        
        glm::mat4 model = tr.GetLocalMatrix();  // + иерархия, если добавлена
        
        mesh.Render(time, model, viewMatrix, projectionMatrix);
    }
}

void Scene::UpdateHierarchy() {
    // TODO: Реализация обхода дерева для глобальных трансформов
    // Пример: Для каждого entity с Hierarchy, вычисляем global = parent.global * local
    // Пока заглушка — предполагаем все трансформы локальные
}

void Scene::UpdatePhysics(float dt) {
    auto physView = registry.view<Transform, PhysicsBody>();
    for (auto e : physView) {
        auto& tr = physView.get<Transform>(e);
        auto& phys = physView.get<PhysicsBody>(e);
        tr.position += phys.velocity * dt;
        // ... добавь collision/силы позже
    }
}

} // namespace ogle