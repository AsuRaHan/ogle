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
    OGLE::ModelEntity* model = GetActiveWorld().GetModel(entity);
    if (!model) {
        return false;
    }

    return model->SetDiffuseTexturePath(texturePath);
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
