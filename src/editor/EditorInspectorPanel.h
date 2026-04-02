#pragma once

#include "managers/WorldManager.h"
#include "managers/PhysicsManager.h"
#include "managers/CameraManager.h"

struct EditorState;

class EditorInspectorPanel
{
public:
    void Draw(EditorState& state, WorldManager& worldManager, PhysicsManager& physicsManager, const CameraManager& cameraManager);
};
