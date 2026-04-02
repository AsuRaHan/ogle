#pragma once

#include "../world/IWorldAccess.h"
#include "../world/World.h"
#include "../world/WorldObject.h"

#include <glm/vec3.hpp>

#include <memory>
#include <string>

/// <summary>
/// WorldManager is responsible for creating, managing and updating the active world instance.
/// It provides convenient helpers for adding entities, primitives, lights and models.
/// </summary>
class WorldManager
    : public IWorldAccess
{
public:
    /// <summary>Initializes a new instance of WorldManager and creates an empty world.</summary>
    WorldManager();

    /// <summary>Creates a new empty world instance.</summary>
    void CreateWorld();
    /// <summary>Populates the active world with a default scene layout.</summary>
    // Deprecated: Use WorldGenerator::GenerateDefaultWorld instead.
    [[deprecated("Use WorldGenerator::GenerateDefaultWorld")]]
    void CreateDefaultWorld();

    /// <summary>Gets a reference to the active world for modification.</summary>
    OGLE::World& GetActiveWorld();
    /// <summary>Gets a const reference to the active world.</summary>
    const OGLE::World& GetActiveWorld() const;
    /// <summary>Creates a world object of specified kind.</summary>
    OGLE::WorldObject CreateWorldObject(
        const std::string& name,
        OGLE::WorldObjectKind kind = OGLE::WorldObjectKind::Generic);
    /// <summary>Retrieves a world object by its entity identifier.</summary>
    OGLE::WorldObject GetWorldObject(OGLE::Entity entity);
    /// <summary>Finds an entity by its name.</summary>
    OGLE::Entity FindEntityByName(const std::string& name) const;

    /// <summary>Adds a model entity to the world.</summary>
    OGLE::Entity AddModel(std::shared_ptr<OGLE::ModelEntity> model, const std::string& name = "Model");
    /// <summary>Loads a model from file and adds it to the world.</summary>
    OGLE::Entity CreateModelFromFile(
        const std::string& filePath,
        OGLE::ModelType type = OGLE::ModelType::DYNAMIC,
        const std::string& name = "Model");
    /// <summary>Creates a primitive entity with given type, position, scale and texture.</summary>
    OGLE::Entity CreatePrimitive(
        const std::string& name,
        OGLE::PrimitiveType type,
        const glm::vec3& position,
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const std::string& diffuseTexturePath = "");
    /// <summary>Creates a directional light entity.</summary>
    OGLE::Entity CreateDirectionalLight(
        const std::string& name,
        const glm::vec3& rotation,
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f,
        bool castShadows = true,
        bool primary = true);
    /// <summary>Creates a point light entity.</summary>
    OGLE::Entity CreatePointLight(
        const std::string& name,
        const glm::vec3& position,
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 2.0f,
        float range = 8.0f);
    /// <summary>Clears all entities from the active world.</summary>
    void ClearWorld();
    /// <summary>Checks if an entity is still valid in the world.</summary>
    bool IsEntityValid(OGLE::Entity entity) const;
    /// <summary>Sets the world transform position of an entity.</summary>
    bool SetEntityPosition(OGLE::Entity entity, const glm::vec3& position);
    /// <summary>Sets the world transform rotation of an entity.</summary>
    bool SetEntityRotation(OGLE::Entity entity, const glm::vec3& rotation);
    /// <summary>Sets the world transform scale of an entity.</summary>
    bool SetEntityScale(OGLE::Entity entity, const glm::vec3& scale);
    /// <summary>Assigns a diffuse texture to an entity.</summary>
    bool SetEntityDiffuseTexture(OGLE::Entity entity, const std::string& texturePath);

    /// <summary>Updates the world state (physics, animations, etc.).</summary>
    void Update();
    /// <summary>Saves the active world to a file.</summary>
    void SaveActiveWorld(const std::string& path);
    /// <summary>Loads a world from a file.</summary>
    void LoadActiveWorld(const std::string& path);

private:
    std::unique_ptr<OGLE::World> m_activeWorld;
};
