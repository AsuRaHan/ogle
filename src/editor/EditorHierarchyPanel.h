#pragma once

struct EditorState;
class WorldManager;
class PhysicsManager;

class EditorHierarchyPanel
{
public:
    void Draw(EditorState& state, WorldManager& worldManager, PhysicsManager& physicsManager);
};
