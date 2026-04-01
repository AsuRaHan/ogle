#pragma once

#include "WorldComponents.h"
#include "WorldObject.h"

#include "../models/ModelEntity.h"

namespace OGLE {
    class World {
    public:
        World();
        ~World();

        WorldObject CreateWorldObject(
            const std::string& name = "WorldObject",
            WorldObjectKind kind = WorldObjectKind::Generic);
        WorldObject CreateMeshObject(const std::string& name = "MeshObject");
        WorldObject GetWorldObject(Entity entity);
        const WorldObject GetWorldObject(Entity entity) const;

        Entity CreateEntity(const std::string& name = "Entity");
        void DestroyEntity(Entity entity);
        Entity FindEntityByName(const std::string& name) const;

        Entity AddModel(std::shared_ptr<ModelEntity> model, const std::string& name = "Model");
        Entity CreateModelFromFile(
            const std::string& filePath,
            ModelType type = ModelType::DYNAMIC,
            const std::string& name = "Model");

        void Update();
        void Draw();
        void Clear();

        void Save(const std::string& path);
        void Load(const std::string& path);

        bool IsValid(Entity entity) const;
        bool HasModel(Entity entity) const;

        WorldObjectComponent* GetWorldObjectComponent(Entity entity);
        const WorldObjectComponent* GetWorldObjectComponent(Entity entity) const;
        NameComponent* GetNameComponent(Entity entity);
        const NameComponent* GetNameComponent(Entity entity) const;
        void SetName(Entity entity, const std::string& name);

        ModelEntity* GetModel(Entity entity);
        const ModelEntity* GetModel(Entity entity) const;

        TransformComponent* GetTransform(Entity entity);
        const TransformComponent* GetTransform(Entity entity) const;
        PhysicsBodyComponent* GetPhysicsBody(Entity entity);
        const PhysicsBodyComponent* GetPhysicsBody(Entity entity) const;

        void SetTransform(
            Entity entity,
            const glm::vec3& position,
            const glm::vec3& rotation,
            const glm::vec3& scale);

        entt::registry& GetRegistry() { return m_registry; }
        const entt::registry& GetRegistry() const { return m_registry; }

    private:
        void SyncModelTransform(Entity entity);

        entt::registry m_registry;
    };
}
