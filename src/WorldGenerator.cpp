// WorldGenerator.cpp
// Implementation of the WorldGenerator utility.

#include "WorldGenerator.h"
#include "./managers/WorldManager.h"
#include <glm/vec3.hpp>

// Note: FileSystem is used in the original CreateDefaultWorld logic.
// We'll include the necessary header to access it.
#include "core/FileSystem.h"

void WorldGenerator::GenerateDefaultWorld(WorldManager& manager)
{
    // The original logic from WorldManager::CreateDefaultWorld is reproduced here.
    const std::string sharedTexturePath = FileSystem::Exists("assets/Q4JOI.jpg")
        ? FileSystem::ResolvePath("assets/Q4JOI.jpg").string()
        : std::string();

    // Ensure a fresh world instance
    manager.CreateWorld();

    manager.CreatePrimitive("CenterBlock", OGLE::PrimitiveType::Cube, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), sharedTexturePath);
    manager.CreatePrimitive("Floor", OGLE::PrimitiveType::Cube, glm::vec3(0.0f, -1.5f, 0.0f), glm::vec3(12.0f, 0.25f, 12.0f), sharedTexturePath);
    manager.CreatePrimitive("NorthPillar", OGLE::PrimitiveType::Cube, glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.75f, 2.5f, 0.75f), sharedTexturePath);
    manager.CreatePrimitive("SouthPillar", OGLE::PrimitiveType::Cube, glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.75f, 1.75f, 0.75f), sharedTexturePath);
    manager.CreatePrimitive("WestBlock", OGLE::PrimitiveType::Cube, glm::vec3(-3.5f, -0.4f, 1.5f), glm::vec3(1.5f, 0.8f, 1.5f), sharedTexturePath);
    manager.CreatePrimitive("EastBlock", OGLE::PrimitiveType::Cube, glm::vec3(3.0f, 1.2f, -1.5f), glm::vec3(1.25f, 2.4f, 1.25f), sharedTexturePath);
    manager.CreatePrimitive("Bridge", OGLE::PrimitiveType::Cube, glm::vec3(0.5f, 2.1f, -1.5f), glm::vec3(3.5f, 0.25f, 0.75f), sharedTexturePath);
    manager.CreatePrimitive("MarkerA", OGLE::PrimitiveType::Cube, glm::vec3(-2.5f, -0.8f, -3.0f), glm::vec3(0.4f, 0.4f, 0.4f), sharedTexturePath);
    manager.CreatePrimitive("MarkerB", OGLE::PrimitiveType::Cube, glm::vec3(2.8f, -0.8f, 3.2f), glm::vec3(0.5f, 0.5f, 0.5f), sharedTexturePath);
    manager.CreateDirectionalLight("Sun", glm::vec3(-50.0f, 45.0f, 0.0f), glm::vec3(1.0f, 0.96f, 0.9f), 2.2f, true, true);
    manager.CreatePointLight("WarmLamp", glm::vec3(-2.0f, 1.8f, 1.5f), glm::vec3(1.0f, 0.75f, 0.45f), 3.0f, 7.0f);
}

// Helper: create a simple floor at given position with specified width and depth
void WorldGenerator::CreateFloor(WorldManager& manager, const glm::vec3& position, float width, float depth, const std::string& texture)
{
    manager.CreatePrimitive("Floor", OGLE::PrimitiveType::Cube, position, glm::vec3(width, 0.1f, depth), texture);
}

// Helper: create a wall segment
void WorldGenerator::CreateWall(WorldManager& manager, const glm::vec3& position, float height, float width, float depth, const std::string& texture)
{
    manager.CreatePrimitive("Wall", OGLE::PrimitiveType::Cube, position, glm::vec3(width, height, depth), texture);
}

// Helper: create an environment object such as a tree or statue
void WorldGenerator::CreateEnvironmentObject(WorldManager& manager, const std::string& name, OGLE::PrimitiveType type, const glm::vec3& position, const std::string& texture)
{
    manager.CreatePrimitive(name, type, position, glm::vec3(1.0f, 1.0f, 1.0f), texture);
}

// Generate a more complex default world with walls and some objects
void WorldGenerator::GenerateComplexWorld(WorldManager& manager)
{
    // Basic floor
    CreateFloor(manager, glm::vec3(0.0f, -1.5f, 0.0f), 12.0f, 12.0f, "assets/Q4JOI.jpg");
    // Four walls around the origin
    const float wallHeight = 3.0f;
    CreateWall(manager, glm::vec3(0.0f, 0.0f, -6.0f), wallHeight, 0.2f, 12.0f, "assets/Q4JOI.jpg");
    CreateWall(manager, glm::vec3(0.0f, 0.0f, 6.0f), wallHeight, 0.2f, 12.0f, "assets/Q4JOI.jpg");
    CreateWall(manager, glm::vec3(-6.0f, 0.0f, 0.0f), wallHeight, 12.0f, 0.2f, "assets/Q4JOI.jpg");
    CreateWall(manager, glm::vec3(6.0f, 0.0f, 0.0f), wallHeight, 12.0f, 0.2f, "assets/Q4JOI.jpg");
    // Some decorative objects inside
    CreateEnvironmentObject(manager, "Tree1", OGLE::PrimitiveType::Cube, glm::vec3(-3.0f, -1.5f, -3.0f), "assets/Q4JOI.jpg");
    CreateEnvironmentObject(manager, "Statue", OGLE::PrimitiveType::Cube, glm::vec3(2.0f, -1.5f, 2.0f), "assets/Q4JOI.jpg");
    // Lights
    manager.CreateDirectionalLight("Sun", glm::vec3(-50.0f, 45.0f, 0.0f), glm::vec3(1.0f, 0.96f, 0.9f), 2.2f, true, true);
    manager.CreatePointLight("WarmLamp", glm::vec3(-2.0f, 1.8f, 1.5f), glm::vec3(1.0f, 0.75f, 0.45f), 3.0f, 7.0f);
}