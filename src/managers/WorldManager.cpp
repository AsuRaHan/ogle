#include "managers/WorldManager.h"

#include "core/FileSystem.h"
#include "managers/PrimitiveFactory.h"

#include <glm/vec3.hpp>

WorldManager::WorldManager()
{
    CreateWorld();
}

void WorldManager::CreateWorld()
{
    m_activeWorld = std::make_unique<OGLE::World>();
}

void WorldManager::CreateDefaultWorld()
{
    const std::string sharedTexturePath = FileSystem::Exists("assets/Q4JOI.jpg")
        ? FileSystem::ResolvePath("assets/Q4JOI.jpg").string()
        : std::string();

    if (!m_activeWorld) {
        CreateWorld();
    } else {
        m_activeWorld->Clear();
    }

    CreateCube(
        "CenterBlock",
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        sharedTexturePath);

    CreateCube(
        "Floor",
        glm::vec3(0.0f, -1.5f, 0.0f),
        glm::vec3(12.0f, 0.25f, 12.0f),
        sharedTexturePath);

    CreateCube(
        "NorthPillar",
        glm::vec3(0.0f, 0.0f, -4.0f),
        glm::vec3(0.75f, 2.5f, 0.75f),
        sharedTexturePath);

    CreateCube(
        "SouthPillar",
        glm::vec3(0.0f, 0.0f, 4.0f),
        glm::vec3(0.75f, 1.75f, 0.75f),
        sharedTexturePath);

    CreateCube(
        "WestBlock",
        glm::vec3(-3.5f, -0.4f, 1.5f),
        glm::vec3(1.5f, 0.8f, 1.5f),
        sharedTexturePath);

    CreateCube(
        "EastBlock",
        glm::vec3(3.0f, 1.2f, -1.5f),
        glm::vec3(1.25f, 2.4f, 1.25f),
        sharedTexturePath);

    CreateCube(
        "Bridge",
        glm::vec3(0.5f, 2.1f, -1.5f),
        glm::vec3(3.5f, 0.25f, 0.75f),
        sharedTexturePath);

    CreateCube(
        "MarkerA",
        glm::vec3(-2.5f, -0.8f, -3.0f),
        glm::vec3(0.4f, 0.4f, 0.4f),
        sharedTexturePath);

    CreateCube(
        "MarkerB",
        glm::vec3(2.8f, -0.8f, 3.2f),
        glm::vec3(0.5f, 0.5f, 0.5f),
        sharedTexturePath);

    CreateDirectionalLight(
        "Sun",
        glm::vec3(-50.0f, 45.0f, 0.0f),
        glm::vec3(1.0f, 0.96f, 0.9f),
        2.2f,
        true,
        true);

    CreatePointLight(
        "WarmLamp",
        glm::vec3(-2.0f, 1.8f, 1.5f),
        glm::vec3(1.0f, 0.75f, 0.45f),
        3.0f,
        7.0f);
}

OGLE::World& WorldManager::GetActiveWorld()
{
    return *m_activeWorld;
}

const OGLE::World& WorldManager::GetActiveWorld() const
{
    return *m_activeWorld;
}

OGLE::WorldObject WorldManager::CreateWorldObject(const std::string& name, OGLE::WorldObjectKind kind)
{
    return GetActiveWorld().CreateWorldObject(name, kind);
}

OGLE::WorldObject WorldManager::GetWorldObject(OGLE::Entity entity)
{
    return GetActiveWorld().GetWorldObject(entity);
}

OGLE::Entity WorldManager::FindEntityByName(const std::string& name) const
{
    return GetActiveWorld().FindEntityByName(name);
}

OGLE::Entity WorldManager::AddModel(std::shared_ptr<OGLE::ModelEntity> model, const std::string& name)
{
    return GetActiveWorld().AddModel(std::move(model), name);
}

OGLE::Entity WorldManager::CreateModelFromFile(
    const std::string& filePath,
    OGLE::ModelType type,
    const std::string& name) {

    return GetActiveWorld().CreateModelFromFile(filePath, type, name);
}

OGLE::Entity WorldManager::CreatePrimitive(
    const std::string& name,
    OGLE::PrimitiveType type,
    const glm::vec3& position,
    const glm::vec3& scale,
    const std::string& diffuseTexturePath)
{
    auto model = PrimitiveFactory::CreatePrimitiveModel(type, diffuseTexturePath);
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

OGLE::Entity WorldManager::CreateCube(
    const std::string& name,
    const glm::vec3& position,
    const glm::vec3& scale,
    const std::string& diffuseTexturePath) {
    return CreatePrimitive(name, OGLE::PrimitiveType::Cube, position, scale, diffuseTexturePath);
}

OGLE::Entity WorldManager::CreateDirectionalLight(
    const std::string& name,
    const glm::vec3& rotation,
    const glm::vec3& color,
    float intensity,
    bool castShadows,
    bool primary)
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

OGLE::Entity WorldManager::CreatePointLight(
    const std::string& name,
    const glm::vec3& position,
    const glm::vec3& color,
    float intensity,
    float range)
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

bool WorldManager::SetEntityDiffuseTexture(OGLE::Entity entity, const std::string& texturePath)
{
    OGLE::World& world = GetActiveWorld();

    if (OGLE::MaterialComponent* material = world.GetMaterial(entity)) {
        return material->material.SetDiffuseTexturePath(texturePath);
    }

    // Ensure mesh entities always have authoring material in ECS.
    if (const OGLE::ModelEntity* model = world.GetModel(entity)) {
        OGLE::MaterialComponent component;
        component.material = model->GetMaterial();
        component.material.SetDiffuseTexturePath(texturePath);
        world.GetRegistry().emplace<OGLE::MaterialComponent>(entity, component);
        return true;
    }

    return false;
}

void WorldManager::Update()
{
    GetActiveWorld().Update();
}

void WorldManager::SaveActiveWorld(const std::string& path)
{
    GetActiveWorld().Save(path);
}

void WorldManager::LoadActiveWorld(const std::string& path)
{
    GetActiveWorld().Load(path);
}
