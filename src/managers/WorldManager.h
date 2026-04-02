#pragma once

#include "../world/World.h"
#include "../world/WorldObject.h"

#include <glm/vec3.hpp>

#include <memory>
#include <string>

class WorldManager
{
public:
    WorldManager();

    void CreateWorld();
    void CreateDefaultWorld();

    OGLE::World& GetActiveWorld();
    const OGLE::World& GetActiveWorld() const;
    OGLE::WorldObject CreateWorldObject(
        const std::string& name,
        OGLE::WorldObjectKind kind = OGLE::WorldObjectKind::Generic);
    OGLE::WorldObject GetWorldObject(OGLE::Entity entity);
    OGLE::Entity FindEntityByName(const std::string& name) const;

    OGLE::Entity AddModel(std::shared_ptr<OGLE::ModelEntity> model, const std::string& name = "Model");
    OGLE::Entity CreateModelFromFile(
        const std::string& filePath,
        OGLE::ModelType type = OGLE::ModelType::DYNAMIC,
        const std::string& name = "Model");
    OGLE::Entity CreateCube(
        const std::string& name,
        const glm::vec3& position,
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const std::string& diffuseTexturePath = "");
    OGLE::Entity CreateDirectionalLight(
        const std::string& name,
        const glm::vec3& rotation,
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f,
        bool castShadows = true,
        bool primary = true);
    OGLE::Entity CreatePointLight(
        const std::string& name,
        const glm::vec3& position,
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 2.0f,
        float range = 8.0f);
    void ClearWorld();
    bool IsEntityValid(OGLE::Entity entity) const;
    bool SetEntityPosition(OGLE::Entity entity, const glm::vec3& position);
    bool SetEntityRotation(OGLE::Entity entity, const glm::vec3& rotation);
    bool SetEntityScale(OGLE::Entity entity, const glm::vec3& scale);
    bool SetEntityDiffuseTexture(OGLE::Entity entity, const std::string& texturePath);

    void Update();
    void SaveActiveWorld(const std::string& path);
    void LoadActiveWorld(const std::string& path);

private:
    std::unique_ptr<OGLE::World> m_activeWorld;
};
