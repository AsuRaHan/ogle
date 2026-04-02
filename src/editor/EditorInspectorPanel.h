#pragma once

struct EditorState;
class WorldManager;
class PhysicsManager;

class EditorInspectorPanel
{
public:
    void Draw(EditorState& state, WorldManager& worldManager, PhysicsManager& physicsManager);
};
