#include "World.h"

#include "core/FileSystem.h"
#include "SceneSerializer.h"
#include "systems/AnimationSystem.h"
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

#include <cmath>
#include <fstream>

namespace OGLE {
    World::World() {
        m_serializer = std::make_unique<SceneSerializer>(*this);
        m_transformSystem = std::make_unique<TransformSystem>(m_registry);
        m_animationSystem = std::make_unique<AnimationSystem>(m_registry);
        m_renderSystem = std::make_unique<RenderSystem>(m_registry);
    }

    World::~World() = default;

    WorldObject World::CreateWorldObject(const std::string& name, WorldObjectKind kind) {
        const Entity entity = m_registry.create();
        m_registry.emplace<WorldObjectComponent>(entity, kind, true, true);
        m_registry.emplace<NameComponent>(entity, name);
        m_registry.emplace<TransformComponent>(entity);
        if (!name.empty()) {
            m_nameToEntityMap[name] = entity;
        }
        return WorldObject(this, entity);
    }

    WorldObject World::CreateMeshObject(const std::string& name) {
        return CreateWorldObject(name, WorldObjectKind::Mesh);
    }

    WorldObject World::GetWorldObject(Entity entity) {
        return WorldObject(this, entity);
    }

    const WorldObject World::GetWorldObject(Entity entity) const {
        return WorldObject(const_cast<World*>(this), entity);
    }

    Entity World::CreateEntity(const std::string& name) {
        return CreateWorldObject(name, WorldObjectKind::Generic).GetEntity();
    }

    void World::DestroyEntity(Entity entity) {
        if (IsValid(entity)) {
            if (const auto* nameComponent = m_registry.try_get<NameComponent>(entity)) {
                if (!nameComponent->value.empty()) {
                    m_nameToEntityMap.erase(nameComponent->value);
                }
            }
            m_registry.destroy(entity);
        }
    }

    Entity World::FindEntityByName(const std::string& name) const {
        auto it = m_nameToEntityMap.find(name);
        if (it != m_nameToEntityMap.end() && IsValid(it->second)) {
            return it->second;
        }

        return entt::null; // Сущность не найдена или стала невалидной
    }

    Entity World::AddModel(std::shared_ptr<ModelEntity> model, const std::string& name) {
        const Entity entity = CreateMeshObject(name).GetEntity();
        auto& transform = m_registry.get<TransformComponent>(entity);
        transform.position = model ? model->GetPosition() : glm::vec3(0.0f);
        transform.rotation = model ? model->GetRotation() : glm::vec3(0.0f);
        transform.scale = model ? model->GetScale() : glm::vec3(1.0f);
        m_registry.emplace<ModelComponent>(entity, std::move(model));
        m_registry.emplace<MaterialComponent>(entity, m_registry.get<ModelComponent>(entity).model ? m_registry.get<ModelComponent>(entity).model->GetMaterial() : Material{ });
        m_registry.emplace<PrimitiveComponent>(entity);
        SyncModelTransform(entity);
        return entity;
    }

    Entity World::CreateModelFromFile(const std::string& filePath, ModelType type, const std::string& name) {
        auto model = std::make_shared<ModelEntity>(type, filePath);
        if (!model->LoadFromFile(filePath)) {
            return entt::null;
        }
        model->BakeToGPU();
        if (!model->GetLoadedDiffuseTexturePath().empty()) {
            model->SetDiffuseTexturePath(model->GetLoadedDiffuseTexturePath());
        }
        const Entity entity = AddModel(std::move(model), name);
        if (m_registry.all_of<PrimitiveComponent>(entity)) {
            auto& primitive = m_registry.get<PrimitiveComponent>(entity);
            primitive.type = PrimitiveType::ModelFile;
            primitive.sourcePath = filePath;
        }

        const ModelEntity* loadedModel = GetModel(entity);
        if (loadedModel) {
            const auto& modelClips = loadedModel->GetAnimationClips();
            if (!modelClips.empty()) {
                if (!m_registry.all_of<AnimationComponent>(entity)) {
                    m_registry.emplace<AnimationComponent>(entity);
                }
                auto& animation = m_registry.get<AnimationComponent>(entity);
                animation.clips = modelClips;
                animation.currentClipIndex = 0;
                animation.currentClip = modelClips[0].name;
                animation.duration = modelClips[0].duration;
                animation.currentTime = 0.0f;
                animation.enabled = true;
            }

            if (loadedModel->GetBoneCount() > 0) {
                SkeletonComponent skeleton;
                skeleton.enabled = true;
                skeleton.boneCount = loadedModel->GetBoneCount();
                skeleton.sourcePath = filePath;
                m_registry.emplace<SkeletonComponent>(entity, skeleton);
            }
        }

        return entity;
    }

    void World::Update(float deltaTime) {
        m_transformSystem->SyncAllModels();
        m_animationSystem->Update(deltaTime);
    }

    void World::Draw() {
        m_renderSystem->Draw();
    }

    void World::Clear() {
        m_registry.clear();
        m_nameToEntityMap.clear();
    }
    
    void World::Save(const std::string& path) {
        m_serializer->Save(path);
    }

    void World::Load(const std::string& path) {
        m_serializer->Load(path);
    }

    bool World::IsValid(Entity entity) const {
        return m_registry.valid(entity);
    }

    bool World::HasModel(Entity entity) const {
        return IsValid(entity) && m_registry.all_of<ModelComponent>(entity);
    }

    void World::SetName(Entity entity, const std::string& name) {
        if (auto* nameComponent = GetComponent<NameComponent>(entity)) {
            if (!nameComponent->value.empty()) {
                m_nameToEntityMap.erase(nameComponent->value);
            }
            nameComponent->value = name;
            if (!name.empty()) {
                m_nameToEntityMap[name] = entity;
            }
        }
    }

    ModelEntity* World::GetModel(Entity entity) {
        if (const auto* component = GetComponent<ModelComponent>(entity)) {
            return component->model.get();
        }
        return nullptr;
    }

    const ModelEntity* World::GetModel(Entity entity) const {
        if (const auto* component = GetComponent<ModelComponent>(entity)) {
            return component->model.get();
        }
        return nullptr;
    }

    TransformComponent* World::GetTransform(Entity entity) {
        return GetComponent<TransformComponent>(entity);
    }

    const TransformComponent* World::GetTransform(Entity entity) const {
        return GetComponent<TransformComponent>(entity);
    }

    PhysicsBodyComponent* World::GetPhysicsBody(Entity entity) {
        return GetComponent<PhysicsBodyComponent>(entity);
    }

    const PhysicsBodyComponent* World::GetPhysicsBody(Entity entity) const {
        return GetComponent<PhysicsBodyComponent>(entity);
    }

    MaterialComponent* World::GetMaterial(Entity entity) {
        return GetComponent<MaterialComponent>(entity);
    }

    const MaterialComponent* World::GetMaterial(Entity entity) const {
        return GetComponent<MaterialComponent>(entity);
    }

    ShaderComponent* World::GetShader(Entity entity) {
        return GetComponent<ShaderComponent>(entity);
    }

    const ShaderComponent* World::GetShader(Entity entity) const {
        return GetComponent<ShaderComponent>(entity);
    }

    PrimitiveComponent* World::GetPrimitive(Entity entity) {
        return GetComponent<PrimitiveComponent>(entity);
    }

    const PrimitiveComponent* World::GetPrimitive(Entity entity) const {
        return GetComponent<PrimitiveComponent>(entity);
    }

    void World::SyncModelTransform(Entity entity) {
        m_transformSystem->SyncModelTransform(entity);
    }

    void World::SetTransform(
        Entity entity,
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale) {

        if (auto* transform = GetComponent<TransformComponent>(entity)) {
            transform->position = position;
            transform->rotation = rotation;
            transform->scale = scale;
            SyncModelTransform(entity);
        }
    }

    void World::MakeModelUnique(Entity entity)
    {
        auto* modelComp = m_registry.try_get<ModelComponent>(entity);
        if (!modelComp || !modelComp->model) {
            return;
        }

        if (modelComp->model.use_count() <= 1) {
            return; // Already unique
        }

        // Create a deep copy of the ModelEntity
        auto originalModel = modelComp->model;

        // Create a new model. The file path is made unique to indicate it's a copy.
        auto newModel = std::make_shared<ModelEntity>(originalModel->GetType(), originalModel->GetFilePath() + " (Unique)");

        // Copy geometry data. SetMeshData will create a new MeshBuffer.
        newModel->SetMeshData(originalModel->GetVertices(), originalModel->GetIndices());

        // Copy material
        newModel->GetMaterial() = originalModel->GetMaterial();

        // Replace the shared_ptr in the component
        modelComp->model = newModel;
    }
}
