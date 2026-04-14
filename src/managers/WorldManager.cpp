#include "managers/WorldManager.h"
#include "WorldGenerator.h"

#include "core/FileSystem.h"
#include "../render/Material.h"
#include "../models/PrimitiveFactory.h"

#include <glm/vec3.hpp>

WorldManager::WorldManager()
{
    CreateWorld();
    // Generate default world using dedicated generator
    //WorldGenerator::GenerateDefaultWorld(*this);
}

void WorldManager::CreateWorld()
{
    m_activeWorld = std::make_unique<OGLE::World>();
}

// The original CreateDefaultWorld logic has been moved to the dedicated WorldGenerator class.
// WorldManager no longer contains world generation code.

OGLE::World& WorldManager::GetActiveWorld()
{
    return *m_activeWorld;
}
// Const overload
const OGLE::World& WorldManager::GetActiveWorld() const
{
    return *m_activeWorld;
}

void WorldManager::ClearWorld()
{
    if (!m_activeWorld) {
        CreateWorld();
        return;
    }

    m_activeWorld->Clear();
}

bool WorldManager::IsEntityValid(OGLE::Entity entity) const
{
    return m_activeWorld && m_activeWorld->IsValid(entity);
}

bool WorldManager::SetEntityPosition(OGLE::Entity entity, const glm::vec3& position)
{
    auto* transform = GetActiveWorld().GetTransform(entity);
    if (!transform) {
        return false;
    }

    GetActiveWorld().SetTransform(entity, position, transform->rotation, transform->scale);
    return true;
}

bool WorldManager::SetEntityRotation(OGLE::Entity entity, const glm::vec3& rotation)
{
    auto* transform = GetActiveWorld().GetTransform(entity);
    if (!transform) {
        return false;
    }

    GetActiveWorld().SetTransform(entity, transform->position, rotation, transform->scale);
    return true;
}

bool WorldManager::SetEntityScale(OGLE::Entity entity, const glm::vec3& scale)
{
    auto* transform = GetActiveWorld().GetTransform(entity);
    if (!transform) {
        return false;
    }

    GetActiveWorld().SetTransform(entity, transform->position, transform->rotation, scale);
    return true;
}

// bool WorldManager::SetEntityDiffuseTexture(OGLE::Entity entity, const std::string& texturePath)
// {
//     OGLE::World& world = GetActiveWorld();

//     if (OGLE::MaterialComponent* material = world.GetMaterial(entity)) {
//         return material->material.SetDiffuseTexturePath(texturePath);
//     }

//     // Ensure mesh entities always have authoring material in ECS.
//     if (const OGLE::ModelEntity* model = world.GetModel(entity)) {
//         OGLE::MaterialComponent component;
//         component.material = model->GetMaterial();
//         component.material.SetDiffuseTexturePath(texturePath);
//         world.GetRegistry().emplace<OGLE::MaterialComponent>(entity, component);
//         return true;
//     }

//     return false;
// }

bool WorldManager::SetEntityShaderProgram(OGLE::Entity entity, const std::string& shaderProgramName)
{
    OGLE::World& world = GetActiveWorld();
    if (OGLE::MaterialComponent* material = world.GetMaterial(entity)) {
        material->material.SetShaderProgram(shaderProgramName);
        return true;
    }

    if (OGLE::ModelEntity* model = world.GetModel(entity)) {
        model->GetMaterial().SetShaderProgram(shaderProgramName);
        return true;
    }

    return false;
}

void WorldManager::Update(float deltaTime)
{
    GetActiveWorld().Update(deltaTime);
}

void WorldManager::SaveActiveWorld(const std::string& path)
{
    GetActiveWorld().Save(path);
}

void WorldManager::LoadActiveWorld(const std::string& path)
{
    GetActiveWorld().Load(path);
}

// ---------- Additional member function implementations ----------
// FindEntityByName
OGLE::Entity WorldManager::FindEntityByName(const std::string& name) const
{
    return GetActiveWorld().FindEntityByName(name);
}

// AddModel
OGLE::Entity WorldManager::AddModel(std::shared_ptr<OGLE::ModelEntity> model, const std::string& name)
{
    return GetActiveWorld().AddModel(std::move(model), name);
}

// CreateModelFromFile
OGLE::Entity WorldManager::CreateModelFromFile(const std::string& filePath, OGLE::ModelType type, const std::string& name)
{
    return GetActiveWorld().CreateModelFromFile(filePath, type, name);
}

// CreatePrimitive
OGLE::Entity WorldManager::CreatePrimitive(const std::string& name, OGLE::PrimitiveType type, const glm::vec3& position, const glm::vec3& scale, const std::string& diffuseTexturePath)
{
    std::unique_ptr<OGLE::Material> material;
    if (!diffuseTexturePath.empty()) {
        material = std::make_unique<OGLE::Material>();
        material->AddTexture("diffuse", diffuseTexturePath);
        material->SetShaderProgram("default");
    }

    auto model = PrimitiveFactory::CreatePrimitiveModel(type, material.get());
    if (!model) {
        return entt::null;
    }

    const OGLE::Entity entity = AddModel(std::move(model), name);
    GetActiveWorld().SetTransform(entity, position, glm::vec3(0.0f, 0.0f, 0.0f), scale);
    if (auto* primitive = GetActiveWorld().GetPrimitive(entity)) {
        primitive->type = type;
        primitive->sourcePath.clear();
    }
    return entity;
}

// CreateWorldObject
OGLE::WorldObject WorldManager::CreateWorldObject(const std::string& name, OGLE::WorldObjectKind kind)
{
    return GetActiveWorld().CreateWorldObject(name, kind);
}

// CreateDirectionalLight
OGLE::Entity WorldManager::CreateDirectionalLight(const std::string& name, const glm::vec3& rotation, const glm::vec3& color, float intensity, bool castShadows, bool primary)
{
    OGLE::WorldObject lightObject = CreateWorldObject(name, OGLE::WorldObjectKind::Light);
    const OGLE::Entity entity = lightObject.GetEntity();
    GetActiveWorld().SetTransform(entity, glm::vec3(0.0f), rotation, glm::vec3(1.0f));
    auto& light = GetActiveWorld().GetRegistry().emplace<OGLE::LightComponent>(entity);
    light.type = OGLE::LightType::Directional;
    light.color = color;
    light.intensity = intensity;
    light.range = 0.0f;
    light.castShadows = castShadows;
    light.primary = primary;
    return entity;
}

// CreatePointLight
OGLE::Entity WorldManager::CreatePointLight(const std::string& name, const glm::vec3& position, const glm::vec3& color, float intensity, float range)
{
    OGLE::WorldObject lightObject = CreateWorldObject(name, OGLE::WorldObjectKind::Light);
    const OGLE::Entity entity = lightObject.GetEntity();
    GetActiveWorld().SetTransform(entity, position, glm::vec3(0.0f), glm::vec3(1.0f));
    auto& light = GetActiveWorld().GetRegistry().emplace<OGLE::LightComponent>(entity);
    light.type = OGLE::LightType::Point;
    light.color = color;
    light.intensity = intensity;
    light.range = range;
    light.castShadows = false;
    light.primary = false;
    return entity;
}

// GetWorldObject
OGLE::WorldObject WorldManager::GetWorldObject(OGLE::Entity entity)
{
    return GetActiveWorld().GetWorldObject(entity);
}

// CreateDefaultWorld (deprecated – kept for backward compatibility)
void WorldManager::CreateDefaultWorld()
{
    // Delegate to the dedicated generator
    WorldGenerator::GenerateDefaultWorld(*this);
}
