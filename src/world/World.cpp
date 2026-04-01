#include "World.h"

#include "core/FileSystem.h"

#include <fstream>

#include <nlohmann/json.hpp>

namespace OGLE {
    World::World() = default;

    World::~World() = default;

    WorldObject World::CreateWorldObject(const std::string& name, WorldObjectKind kind) {
        const Entity entity = m_registry.create();
        m_registry.emplace<WorldObjectComponent>(entity, kind, true, true);
        m_registry.emplace<NameComponent>(entity, name);
        m_registry.emplace<TransformComponent>(entity);
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
            m_registry.destroy(entity);
        }
    }

    Entity World::FindEntityByName(const std::string& name) const {
        auto view = m_registry.view<NameComponent>();
        for (auto entity : view) {
            const auto& nameComponent = view.get<NameComponent>(entity);
            if (nameComponent.value == name) {
                return entity;
            }
        }

        return entt::null;
    }

    Entity World::AddModel(std::shared_ptr<ModelEntity> model, const std::string& name) {
        const Entity entity = CreateMeshObject(name).GetEntity();
        auto& transform = m_registry.get<TransformComponent>(entity);
        transform.position = model ? model->GetPosition() : glm::vec3(0.0f);
        transform.rotation = model ? model->GetRotation() : glm::vec3(0.0f);
        transform.scale = model ? model->GetScale() : glm::vec3(1.0f);
        m_registry.emplace<ModelComponent>(entity, std::move(model));
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
        return AddModel(std::move(model), name);
    }

    void World::Update() {
        auto view = m_registry.view<TransformComponent, ModelComponent>();
        for (auto entity : view) {
            SyncModelTransform(entity);
        }
    }

    void World::Draw() {
        auto view = m_registry.view<ModelComponent>();
        for (auto entity : view) {
            auto& model = view.get<ModelComponent>(entity);
            if (model.model) {
                model.model->Draw();
            }
        }
    }

    void World::Clear() {
        m_registry.clear();
    }

    void World::Save(const std::string& path) {
        nlohmann::json j;
        auto view = m_registry.view<NameComponent, TransformComponent, ModelComponent>();
        for (auto entity : view) {
            auto& name = view.get<NameComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);
            auto& model = view.get<ModelComponent>(entity);

            if (!model.model) {
                continue;
            }

            nlohmann::json entityJson;
            entityJson["name"] = name.value;
            entityJson["position"] = { transform.position.x, transform.position.y, transform.position.z };
            entityJson["rotation"] = { transform.rotation.x, transform.rotation.y, transform.rotation.z };
            entityJson["scale"] = { transform.scale.x, transform.scale.y, transform.scale.z };

            nlohmann::json modelJson;
            model.model->ToJson(modelJson);
            entityJson["model"] = modelJson;

            j["entities"].push_back(entityJson);
        }

        FileSystem::EnsureParentDirectory(path);
        std::ofstream file(path);
        file << j.dump(4);
    }

    void World::Load(const std::string& path) {
        m_registry.clear();

        const std::filesystem::path resolvedPath = FileSystem::ResolvePath(path);
        std::ifstream file(resolvedPath);
        if (!file.is_open()) {
            return;
        }

        nlohmann::json j;
        file >> j;

        if (!j.contains("entities")) {
            return;
        }

        for (const auto& entityJson : j["entities"]) {
            auto model = std::make_shared<ModelEntity>();
            model->FromJson(entityJson.at("model"));

            const std::string name = entityJson.value("name", "Model");
            const Entity entity = AddModel(model, name);

            auto& transform = m_registry.get<TransformComponent>(entity);
            const auto& positionJson = entityJson.at("position");
            const auto& rotationJson = entityJson.at("rotation");
            const auto& scaleJson = entityJson.at("scale");
            transform.position = glm::vec3(positionJson[0], positionJson[1], positionJson[2]);
            transform.rotation = glm::vec3(rotationJson[0], rotationJson[1], rotationJson[2]);
            transform.scale = glm::vec3(scaleJson[0], scaleJson[1], scaleJson[2]);
            SyncModelTransform(entity);
        }
    }

    bool World::IsValid(Entity entity) const {
        return m_registry.valid(entity);
    }

    bool World::HasModel(Entity entity) const {
        return IsValid(entity) && m_registry.all_of<ModelComponent>(entity);
    }

    WorldObjectComponent* World::GetWorldObjectComponent(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<WorldObjectComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<WorldObjectComponent>(entity);
    }

    const WorldObjectComponent* World::GetWorldObjectComponent(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<WorldObjectComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<WorldObjectComponent>(entity);
    }

    NameComponent* World::GetNameComponent(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<NameComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<NameComponent>(entity);
    }

    const NameComponent* World::GetNameComponent(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<NameComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<NameComponent>(entity);
    }

    void World::SetName(Entity entity, const std::string& name) {
        NameComponent* nameComponent = GetNameComponent(entity);
        if (nameComponent) {
            nameComponent->value = name;
        }
    }

    ModelEntity* World::GetModel(Entity entity) {
        if (!HasModel(entity)) {
            return nullptr;
        }

        auto& component = m_registry.get<ModelComponent>(entity);
        return component.model.get();
    }

    const ModelEntity* World::GetModel(Entity entity) const {
        if (!HasModel(entity)) {
            return nullptr;
        }

        const auto& component = m_registry.get<ModelComponent>(entity);
        return component.model.get();
    }

    TransformComponent* World::GetTransform(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<TransformComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<TransformComponent>(entity);
    }

    const TransformComponent* World::GetTransform(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<TransformComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<TransformComponent>(entity);
    }

    PhysicsBodyComponent* World::GetPhysicsBody(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<PhysicsBodyComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<PhysicsBodyComponent>(entity);
    }

    const PhysicsBodyComponent* World::GetPhysicsBody(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<PhysicsBodyComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<PhysicsBodyComponent>(entity);
    }

    void World::SetTransform(
        Entity entity,
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale) {

        TransformComponent* transform = GetTransform(entity);
        if (!transform) {
            return;
        }

        transform->position = position;
        transform->rotation = rotation;
        transform->scale = scale;
        SyncModelTransform(entity);
    }

    void World::SyncModelTransform(Entity entity) {
        if (!m_registry.all_of<TransformComponent, ModelComponent>(entity)) {
            return;
        }

        auto& transform = m_registry.get<TransformComponent>(entity);
        auto& model = m_registry.get<ModelComponent>(entity);
        if (!model.model) {
            return;
        }

        model.model->SetPosition(transform.position);
        model.model->SetRotation(transform.rotation);
        model.model->SetScale(transform.scale);
    }
}
