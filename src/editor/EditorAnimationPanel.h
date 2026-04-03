#pragma once

#include "editor/EditorState.h"
#include "managers/WorldManager.h"

namespace OGLE {
    class World;
}

class EditorAnimationPanel
{
public:
    void Draw(EditorState& state, WorldManager& worldManager);
};
