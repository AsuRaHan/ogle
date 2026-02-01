// src/scene/Scene.cpp
#include "Scene.h"
#include "render/LightContext.h"
#define GLM_ENABLE_EXPERIMENTAL
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

        // Задаём радиус ограничивающей сферы для куба (sqrt(3)/2 * side ≈ 0.866)
        auto& bounds = registry.get<Bounds>(testEntity);
        bounds.center = glm::vec3(0.0f);
        bounds.radius = 0.866f;

        // Источник направленного света по умолчанию (солнце)
        auto lightEntity = CreateEntity("DirectionalLight");
        registry.emplace<DirectionalLight>(lightEntity);
        auto* lightTr = registry.try_get<Transform>(lightEntity);
        if (lightTr) {
            lightTr->rotation = glm::quat(glm::vec3(glm::radians(-45.0f), glm::radians(30.0f), 0.0f));
            lightTr->MarkDirty();
        }

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

    void Scene::Clear() {
        std::vector<entt::entity> toDestroy;
        for (auto e : registry.view<Transform>()) {
            toDestroy.push_back(e);
        }
        for (entt::entity e : toDestroy) {
            registry.destroy(e);
        }
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

        // EnTT view<T> возвращает только точный тип T; Mesh наследует SceneComponent, но хранится как Mesh
        auto meshView = registry.view<Mesh>();
        for (auto e : meshView) {
            if (registry.valid(e)) {
                meshView.get<Mesh>(e).Update(deltaTime);
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
        }

        if (auto* hier = registry.try_get<Hierarchy>(e)) {
            for (auto child : hier->children) {
                UpdateTransformHierarchy(child);
                if (auto* bounds = registry.try_get<Bounds>(e)) {
                    if (auto* childBounds = registry.try_get<Bounds>(child)) {
                        float dist = glm::length(childBounds->center - bounds->center);
                        bounds->globalRadius = std::max(bounds->globalRadius, dist + childBounds->globalRadius);
                    }
                }
            }
        }
    }

    void Scene::CollectLights(LightContext& out) const {
        out.numDirectional = 0;
        out.numPoint = 0;
        out.numSpot = 0;

        for (auto e : registry.view<Transform, DirectionalLight>()) {
            if (out.numDirectional >= kMaxDirectionalLights) break;
            auto* tr = registry.try_get<Transform>(e);
            auto* light = registry.try_get<DirectionalLight>(e);
            if (!tr || !light) continue;
            glm::vec3 worldDir;
            if (light->useTransformDirection) {
                worldDir = glm::normalize(glm::vec3(tr->globalMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
            } else {
                worldDir = glm::normalize(glm::vec3(tr->globalMatrix * glm::vec4(light->direction, 0.0f)));
            }
            out.directional[out.numDirectional].direction = worldDir;
            out.directional[out.numDirectional].color = light->color;
            out.directional[out.numDirectional].intensity = light->intensity;
            out.numDirectional++;
        }

        for (auto e : registry.view<Transform, PointLight>()) {
            if (out.numPoint >= kMaxPointLights) break;
            auto* tr = registry.try_get<Transform>(e);
            auto* light = registry.try_get<PointLight>(e);
            if (!tr || !light) continue;
            glm::vec3 pos = glm::vec3(tr->globalMatrix[3]);
            out.point[out.numPoint].position = pos;
            out.point[out.numPoint].color = light->color;
            out.point[out.numPoint].intensity = light->intensity;
            out.point[out.numPoint].constant = light->constant;
            out.point[out.numPoint].linear = light->linear;
            out.point[out.numPoint].quadratic = light->quadratic;
            out.numPoint++;
        }

        for (auto e : registry.view<Transform, SpotLight>()) {
            if (out.numSpot >= kMaxSpotLights) break;
            auto* tr = registry.try_get<Transform>(e);
            auto* light = registry.try_get<SpotLight>(e);
            if (!tr || !light) continue;
            glm::vec3 pos = glm::vec3(tr->globalMatrix[3]);
            glm::vec3 dir = glm::normalize(glm::vec3(tr->globalMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
            out.spot[out.numSpot].position = pos;
            out.spot[out.numSpot].direction = dir;
            out.spot[out.numSpot].color = light->color;
            out.spot[out.numSpot].intensity = light->intensity;
            out.spot[out.numSpot].innerAngle = light->innerAngle;
            out.spot[out.numSpot].outerAngle = light->outerAngle;
            out.spot[out.numSpot].constant = light->constant;
            out.spot[out.numSpot].linear = light->linear;
            out.spot[out.numSpot].quadratic = light->quadratic;
            out.numSpot++;
        }
    }

    void Scene::Render(float time, Camera* camera) {
        if (!camera) return;

        auto viewMatrix = camera->GetViewMatrix();
        auto projectionMatrix = camera->GetProjectionMatrix();

        LightContext lightContext;
        CollectLights(lightContext);

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
            RenderSubtree(root, time, viewMatrix, projectionMatrix, camera, &lightContext);
        }
    }

    void Scene::RenderSubtree(entt::entity e, float time, const glm::mat4& view, const glm::mat4& proj, Camera* cam, const LightContext* lights) {
        if (!registry.valid(e)) return;

        auto* tr = registry.try_get<Transform>(e);
        if (!tr) return;

        glm::vec3 globalPos = glm::vec3(tr->globalMatrix[3]);

        float radius = 1.0f;
        if (auto* bounds = registry.try_get<Bounds>(e)) {
            radius = bounds->globalRadius;
        }

        //if (!cam->IsInFrustum(globalPos, radius)) {
        //    return;
        //}

        if (auto* mesh = registry.try_get<Mesh>(e)) {
            if (mesh->visible) {
                mesh->Render(time, tr->globalMatrix, view, proj, lights);
            }
        }

        if (auto* hier = registry.try_get<Hierarchy>(e)) {
            for (auto child : hier->children) {
                RenderSubtree(child, time, view, proj, cam, lights);
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