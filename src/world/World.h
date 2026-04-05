#pragma once

#include "WorldComponents.h"
#include "WorldObject.h"

#include "../models/ModelEntity.h"

namespace OGLE {
    class SceneSerializer;
    class TransformSystem;
    class AnimationSystem;
    class RenderSystem;


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

        void Clear();

        void Update(float deltaTime);
        void Draw();
        void Save(const std::string& path);
        void Load(const std::string& path);

        void MakeModelUnique(Entity entity);

        bool IsValid(Entity entity) const;
        bool HasModel(Entity entity) const;

        template<typename T>
        T* GetComponent(Entity entity) {
            if (!IsValid(entity) || !m_registry.all_of<T>(entity)) {
                return nullptr;
            }
            return &m_registry.get<T>(entity);
        }

        template<typename T>
        const T* GetComponent(Entity entity) const {
            if (!IsValid(entity) || !m_registry.all_of<T>(entity)) {
                return nullptr;
            }
            return &m_registry.get<T>(entity);
        }

        void SetName(Entity entity, const std::string& name);

        ModelEntity* GetModel(Entity entity);
        const ModelEntity* GetModel(Entity entity) const;

        TransformComponent* GetTransform(Entity entity);
        const TransformComponent* GetTransform(Entity entity) const;

        PhysicsBodyComponent* GetPhysicsBody(Entity entity);
        const PhysicsBodyComponent* GetPhysicsBody(Entity entity) const;

        MaterialComponent* GetMaterial(Entity entity);
        const MaterialComponent* GetMaterial(Entity entity) const;

        ShaderComponent* GetShader(Entity entity);
        const ShaderComponent* GetShader(Entity entity) const;

        PrimitiveComponent* GetPrimitive(Entity entity);
        const PrimitiveComponent* GetPrimitive(Entity entity) const;

        void SyncModelTransform(Entity entity);

        void SetTransform(
            Entity entity,
            const glm::vec3& position,
            const glm::vec3& rotation,
            const glm::vec3& scale);

        entt::registry& GetRegistry() { return m_registry; }
        const entt::registry& GetRegistry() const { return m_registry; }

    private:
        friend class SceneSerializer;

        entt::registry m_registry;
        // Ускоряет поиск сущностей по имени. Заполняется при создании/загрузке/переименовании.
        std::unordered_map<std::string, Entity> m_nameToEntityMap;

        std::unique_ptr<SceneSerializer> m_serializer;
        std::unique_ptr<TransformSystem> m_transformSystem;
        std::unique_ptr<AnimationSystem> m_animationSystem;
        std::unique_ptr<RenderSystem> m_renderSystem;
    };
}
