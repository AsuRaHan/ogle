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

        // Пример: тестовый объект
        auto testEntity = CreateEntity("TestMesh");
        auto& mesh = registry.emplace<Mesh>(testEntity);
        mesh.Initialize();
        mesh.CreateCube();

        // Исправление: REPLACE вместо emplace (если Bounds уже есть от CreateEntity)
        registry.replace<Bounds>(testEntity, glm::vec3(0.0f), 0.866f);

        // Пример иерархии (раскомментируй для теста)
        // auto child = CreateEntity("Child");
        // SetParent(child, testEntity);
    }

    entt::entity Scene::CreateEntity(const std::string& name) {
        auto e = registry.create();
        registry.emplace<Transform>(e);
        registry.emplace<Bounds>(e);  // Default bounds
        if (!name.empty()) {
            registry.emplace<Tag>(e, name);
        }
        return e;
    }

    void Scene::DestroyEntity(entt::entity e) {
        if (!registry.valid(e)) return;

        if (auto* hier = registry.try_get<Hierarchy>(e)) {
            for (auto child : hier->children) {
                DestroyEntity(child);
            }
        }
        registry.destroy(e);
    }

    void Scene::SetParent(entt::entity child, entt::entity parent) {
        if (!registry.valid(child) || !registry.valid(parent)) return;

        // Удаляем старого родителя
        if (auto* hier = registry.try_get<Hierarchy>(child)) {
            if (registry.valid(hier->parent)) {
                if (auto* oldParentHier = registry.try_get<Hierarchy>(hier->parent)) {
                    auto it = std::find(oldParentHier->children.begin(), oldParentHier->children.end(), child);
                    if (it != oldParentHier->children.end()) {
                        oldParentHier->children.erase(it);
                    }
                }
            }
        }

        // Новый parent
        auto& newHier = registry.emplace_or_replace<Hierarchy>(child);
        newHier.parent = parent;

        auto& parentHier = registry.get_or_emplace<Hierarchy>(parent);
        parentHier.children.push_back(child);

        // Dirty
        if (auto* tr = registry.try_get<Transform>(child)) {
            tr->MarkDirty();
        }
    }

    void Scene::Update(float deltaTime) {
        UpdateHierarchy();
        UpdatePhysics(deltaTime);

        auto compView = registry.view<SceneComponent>();
        for (auto e : compView) {
            if (registry.valid(e)) {
                compView.get<SceneComponent>(e).Update(deltaTime);
            }
        }
    }

    void Scene::UpdateHierarchy() {
        std::vector<entt::entity> roots;
        for (auto e : registry.view<Transform>()) {
            if (!registry.valid(e)) continue;
            bool isRoot = true;
            if (auto* hier = registry.try_get<Hierarchy>(e)) {
                isRoot = (hier->parent == entt::null);
            }
            if (isRoot) roots.push_back(e);
        }

        for (auto root : roots) {
            UpdateTransformHierarchy(root);
        }
    }

    void Scene::UpdateTransformHierarchy(entt::entity e) {
        if (!registry.valid(e)) return;

        auto* tr = registry.try_get<Transform>(e);
        if (!tr || !tr->dirty) return;

        glm::mat4 parentGlobal = glm::mat4(1.0f);
        if (auto* hier = registry.try_get<Hierarchy>(e)) {
            if (registry.valid(hier->parent)) {
                auto* parentTr = registry.try_get<Transform>(hier->parent);
                if (parentTr) parentGlobal = parentTr->globalMatrix;
            }
        }

        tr->globalMatrix = parentGlobal * tr->GetLocalMatrix();
        tr->dirty = false;

        if (auto* bounds = registry.try_get<Bounds>(e)) {
            bounds->globalRadius = bounds->radius;

            if (auto* hier = registry.try_get<Hierarchy>(e)) {
                for (auto child : hier->children) {
                    UpdateTransformHierarchy(child);
                    if (auto* childBounds = registry.try_get<Bounds>(child)) {
                        float dist = glm::length(childBounds->center - bounds->center);
                        bounds->globalRadius = std::max(bounds->globalRadius, dist + childBounds->globalRadius);
                    }
                }
            }
        }

        if (auto* hier = registry.try_get<Hierarchy>(e)) {
            for (auto child : hier->children) {
                UpdateTransformHierarchy(child);
            }
        }
    }

    void Scene::Render(float time, Camera* camera) {
        if (!camera) return;

        auto viewMatrix = camera->GetViewMatrix();
        auto projectionMatrix = camera->GetProjectionMatrix();

        std::vector<entt::entity> roots;
        for (auto e : registry.view<Transform>()) {
            if (!registry.valid(e)) continue;
            bool isRoot = true;
            if (auto* hier = registry.try_get<Hierarchy>(e)) {
                isRoot = (hier->parent == entt::null);
            }
            if (isRoot) roots.push_back(e);
        }

        for (auto root : roots) {
            RenderSubtree(root, time, viewMatrix, projectionMatrix, camera);
        }
    }

    void Scene::RenderSubtree(entt::entity e, float time, const glm::mat4& view, const glm::mat4& proj, Camera* cam) {
        if (!registry.valid(e)) return;

        auto* tr = registry.try_get<Transform>(e);
        if (!tr) return;

        glm::vec3 globalPos = glm::vec3(tr->globalMatrix[3]);

        float radius = 1.0f;
        if (auto* bounds = registry.try_get<Bounds>(e)) {
            radius = bounds->globalRadius;
        }

        if (!cam->IsInFrustum(globalPos, radius)) {
            return;
        }

        if (auto* mesh = registry.try_get<Mesh>(e)) {
            if (mesh->visible) {
                mesh->Render(time, tr->globalMatrix, view, proj);
            }
        }

        if (auto* hier = registry.try_get<Hierarchy>(e)) {
            for (auto child : hier->children) {
                RenderSubtree(child, time, view, proj, cam);
            }
        }
    }

    void Scene::UpdatePhysics(float dt) {
        auto physView = registry.view<Transform, PhysicsBody>();
        //for (auto e : physView) {
        //    if (!registry.valid(e)) continue;
        //    auto& tr = physView.get<Transform>(e);  // Safe, view гарантирует наличие
        //    auto& phys = physView.get<PhysicsBody>(e);
        //    tr.position += phys.velocity * dt;
        //    tr.MarkDirty();
        //}
    }

} // namespace ogle