// WorldGenerator.h
// Utility class for generating default world layouts.
// It encapsulates the logic that was previously in WorldManager::CreateDefaultWorld.

#pragma once

#include "managers/WorldManager.h"

class WorldGenerator
{
public:
    // Generates a default world layout using the provided WorldManager.
    static void GenerateDefaultWorld(WorldManager& manager);
    // New helper methods for more complex world generation
    static void CreateFloor(WorldManager& manager, const glm::vec3& position, float width, float depth, const std::string& texture);
    static void CreateWall(WorldManager& manager, const glm::vec3& position, float height, float width, float depth, const std::string& texture);
    static void CreateEnvironmentObject(WorldManager& manager, const std::string& name, OGLE::PrimitiveType type, const glm::vec3& position, const std::string& texture);
    static void GenerateComplexWorld(WorldManager& manager);
};
