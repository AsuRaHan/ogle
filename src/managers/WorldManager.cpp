#include "managers/WorldManager.h"

#include "core/FileSystem.h"

#include <glm/vec3.hpp>
#include <vector>

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

OGLE::Entity WorldManager::CreateCube(
    const std::string& name,
    const glm::vec3& position,
    const glm::vec3& scale,
    const std::string& diffuseTexturePath) {

    static const std::vector<float> vertices = {
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f
    };

    static const std::vector<unsigned int> indices = {
         0,  1,  2,   2,  3,  0,
         4,  6,  5,   6,  4,  7,
         8,  9, 10,  10, 11,  8,
        12, 14, 13,  14, 12, 15,
        16, 17, 18,  18, 19, 16,
        20, 22, 21,  22, 20, 23
    };

    auto model = std::make_shared<OGLE::ModelEntity>(OGLE::ModelType::STATIC);
    model->SetMeshData(vertices, indices);
    if (!diffuseTexturePath.empty()) {
        model->SetDiffuseTexturePath(diffuseTexturePath);
    }

    const OGLE::Entity entity = AddModel(model, name);
    GetActiveWorld().SetTransform(entity, position, glm::vec3(0.0f, 0.0f, 0.0f), scale);
    if (auto* primitive = GetActiveWorld().GetPrimitive(entity)) {
        primitive->type = OGLE::PrimitiveType::Cube;
        primitive->sourcePath.clear();
    }
    return entity;
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
    bool updated = false;

    if (OGLE::MaterialComponent* material = GetActiveWorld().GetMaterial(entity)) {
        updated = material->material.SetDiffuseTexturePath(texturePath);
    }

    OGLE::ModelEntity* model = GetActiveWorld().GetModel(entity);
    if (model) {
        updated = model->SetDiffuseTexturePath(texturePath) || updated;
    }

    if (!updated) {
        return false;
    }
    return true;
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
